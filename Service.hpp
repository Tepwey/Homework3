#pragma once

#include "Repository.hpp"

class ProcurementService {
private:
    PurchaseRequestRepository& requests;
    BudgetUpdater& budgets;
    BudgetId budget;

public:
    ProcurementService(PurchaseRequestRepository& request_repo, BudgetUpdater& budget_updater, BudgetId budget_id)
        : requests(request_repo), budgets(budget_updater), budget(budget_id) {}

    void add_line(RequestId request, LineId line, Product product, Quantity quantity) {
        requests.update(request, [line, product = std::move(product), quantity](PurchaseRequest& r) {r.add_line(line, product, quantity);});
    }

    void remove_line(RequestId request, LineId line) {
        requests.update(request,
            [line](PurchaseRequest& r) { r.remove_line(line); });
    }

    void submit(RequestId request) {
        auto snapshot = requests.load(request);
        if (!snapshot) throw RequestNotFound(request.value);
        snapshot->submit();
        const Money amount(snapshot->total());
        budgets.update(budget, [amount](DepartmentBudget& b) { b.spend(amount); });
        try {
            requests.update(request, [amount](PurchaseRequest& r) {
                r.submit();
                if (r.total() != amount.GetKopecks())
                throw RequestChanged(r.GetId().value, amount.GetKopecks(), r.total());
            });
        } catch(...) {
            budgets.update(budget, [amount](DepartmentBudget& b) { b.refund(amount); });
            throw;
        }
    }

    void cancel(RequestId request, AuthorId actor) {
        Money amount(0);
        requests.update(request, [&amount, actor](PurchaseRequest& r) {
            r.cancel(actor);
            amount = Money(r.total());
        });
        budgets.update(budget, [amount](DepartmentBudget& b) { b.refund(amount); });
    }
};