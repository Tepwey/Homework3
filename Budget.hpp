#pragma once

#include "Errors.hpp"
#include "Values.hpp"

struct BudgetId { std::int64_t value; };

class DepartmentBudget {
private:
    BudgetId id;
    Money limit;
    Money spent;

public:
    DepartmentBudget(BudgetId budget_id, Money budget_limit)
        : id(budget_id), limit(budget_limit), spent(Money(0)) {
            if (limit.GetKopecks() < 0) {
            throw InvalidBudgetLimit(id.value, limit.GetKopecks());
            }
    }

    BudgetId GetId() const {
        return id;
    }

    Money GetLimit() const {
        return limit;
    }

    Money GetSpent() const {
        return spent;
    }

    Money GetRemaining() const {
        return Money(limit.GetKopecks() - spent.GetKopecks());
    }

    void spend(Money amount) {
        if (amount.GetKopecks() <= 0)
            throw InvalidBudgetAmount(id.value, amount.GetKopecks());
        std::int64_t remaining = limit.GetKopecks() - spent.GetKopecks();
        if (amount.GetKopecks() > remaining)
            throw BudgetExceeded(id.value, remaining, amount.GetKopecks());
        spent = Money(spent.GetKopecks() + amount.GetKopecks());
    }

    void refund(Money amount) {
        if (amount.GetKopecks() <= 0)
            throw InvalidBudgetAmount(id.value, amount.GetKopecks());
        if (amount.GetKopecks() > spent.GetKopecks())
            throw RefundExceedsSpent(id.value, spent.GetKopecks(), amount.GetKopecks());
        spent = Money(spent.GetKopecks() - amount.GetKopecks());
    }
};