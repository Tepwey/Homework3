#include <iostream>
 
#include "Service.hpp"
 
static int failed = 0;
 
#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            ++failed; \
            std::cerr << "FAIL строка " << __LINE__ << ": " #cond "\n";\
        } \
    } while (0)\


#define CHECK_THROWS(expr, Type)\
    do {\
        bool matched = false;\
        try { expr; } catch (const Type&) { matched = true; } catch (...) {}\
        if (!matched) {\
            ++failed;\
            std::cerr << "FAIL строка " << __LINE__\
                      << ": ожидалось " #Type "\n";\
        }\
    } while (0)\
 
static Product prod(const char* code, std::int64_t rub) {
    return Product(code, Money(rub * 100));
}
 

static PurchaseRequest make_draft(std::int64_t request_id = 1) {
    PurchaseRequest r(RequestId{request_id}, AuthorId{7});
    r.add_line(LineId{1}, prod("A", 100), Quantity(10));
    r.add_line(LineId{2}, prod("B", 500), Quantity(2));
    return r;
}
 

struct Env {
    InMemoryPurchaseRequestRepository requests;
    InMemoryBudgetRepository budgets;
    ProcurementService service;
 
    explicit Env(std::int64_t budget_rub) : service(requests, budgets, BudgetId{1}) {
        budgets.create(DepartmentBudget(BudgetId{1}, Money(budget_rub * 100)));
    }
    std::int64_t remaining_rub() {
        return budgets.load(BudgetId{1})->GetRemaining().GetKopecks() / 100;
    }
    RequestStatus status(std::int64_t id) {
        return requests.load(RequestId{id})->GetStatus();
    }
};
 

static void test_remove_line_basic() {
    auto r = make_draft();
    r.remove_line(LineId{1});
    CHECK(r.GetLines().size() == 1);
    CHECK(r.GetLines()[0].GetId().value == 2);
    CHECK(r.total() == 1000 * 100);
}
 
static void test_remove_line_not_found() {
    auto r = make_draft();
    CHECK_THROWS(r.remove_line(LineId{999}), LineNotFound);
    CHECK(r.GetLines().size() == 2);
}
 
static void test_remove_line_only_in_draft() {
    auto submitted = make_draft();
    submitted.submit();
    CHECK_THROWS(submitted.remove_line(LineId{1}), RequestNotDraft);
    CHECK(submitted.GetLines().size() == 2);
 
    auto cancelled = make_draft();
    cancelled.submit();
    cancelled.cancel(AuthorId{7});
    CHECK_THROWS(cancelled.remove_line(LineId{1}), RequestNotDraft);
    CHECK(cancelled.GetLines().size() == 2);
}
 
static void test_remove_last_line_then_submit() {
    auto r = make_draft();
    r.remove_line(LineId{1});
    r.remove_line(LineId{2});
    CHECK(r.GetLines().empty());
    CHECK(r.GetStatus() == RequestStatus::draft);
    CHECK_THROWS(r.submit(), EmptyRequest);
}
 
static void test_remove_line_frees_code_and_id() {
    auto r = make_draft();
    r.remove_line(LineId{1});
    r.add_line(LineId{1}, prod("A", 120), Quantity(1));
    CHECK(r.GetLines().size() == 2);
}
 
static void test_remove_line_through_service() {
    Env env(10'000);
    env.requests.create(make_draft());
    env.service.remove_line(RequestId{1}, LineId{1});
    CHECK(env.requests.load(RequestId{1})->GetLines().size() == 1);
 
    CHECK_THROWS(env.service.remove_line(RequestId{1}, LineId{999}), LineNotFound);
    CHECK(env.requests.load(RequestId{1})->GetLines().size() == 1);
 
    CHECK_THROWS(env.service.remove_line(RequestId{404}, LineId{1}), RequestNotFound);
}
 

static void test_submit_spends_budget() {
    Env env(10'000);
    env.requests.create(make_draft());
    env.service.submit(RequestId{1});
    CHECK(env.status(1) == RequestStatus::submitted);
    CHECK(env.remaining_rub() == 8000);
}
 
static void test_submit_refused_when_not_enough() {
    Env env(1500);
    env.requests.create(make_draft());
    CHECK_THROWS(env.service.submit(RequestId{1}), BudgetExceeded);
    CHECK(env.status(1) == RequestStatus::draft);
    CHECK(env.remaining_rub() == 1500);
}
 
static void test_submit_exactly_remaining() {
    Env env(2000);
    env.requests.create(make_draft());
    env.service.submit(RequestId{1});
    CHECK(env.remaining_rub() == 0);
}
 
static void test_drafts_are_not_limited() {
    Env env(2000);
    env.requests.create(make_draft(1));
    env.requests.create(make_draft(2));
    env.service.submit(RequestId{1});

    env.service.add_line(RequestId{2}, LineId{9}, prod("Z", 5), Quantity(1));
    env.service.remove_line(RequestId{2}, LineId{9});


    CHECK_THROWS(env.service.submit(RequestId{2}), BudgetExceeded);
    CHECK(env.status(2) == RequestStatus::draft);
}
 
static void test_cancel_refunds_budget() {
    Env env(3000);
    env.requests.create(make_draft(1));
    env.requests.create(make_draft(2));
 
    env.service.submit(RequestId{1});
    CHECK_THROWS(env.service.submit(RequestId{2}), BudgetExceeded);
 
    env.service.cancel(RequestId{1}, AuthorId{7});
    CHECK(env.status(1) == RequestStatus::cancelled);
    CHECK(env.remaining_rub() == 3000);
 
    env.service.submit(RequestId{2});
    CHECK(env.remaining_rub() == 1000);
}
 
static void test_cancel_by_stranger_keeps_money_spent() {
    Env env(10'000);
    env.requests.create(make_draft());
    env.service.submit(RequestId{1});
    CHECK_THROWS(env.service.cancel(RequestId{1}, AuthorId{8}), CancellationDenied);
    CHECK(env.status(1) == RequestStatus::submitted);
    CHECK(env.remaining_rub() == 8000);
}
 
static void test_failed_submit_does_not_spend() {
    Env env(10'000);
    env.requests.create(PurchaseRequest(RequestId{3}, AuthorId{7}));
    CHECK_THROWS(env.service.submit(RequestId{3}), EmptyRequest);
    CHECK(env.remaining_rub() == 10'000);
 
    env.requests.create(make_draft());
    env.service.submit(RequestId{1});
    CHECK_THROWS(env.service.submit(RequestId{1}), RequestNotDraft);
    CHECK(env.remaining_rub() == 8000);
}
 
static void test_budget_aggregate() {
    DepartmentBudget b(BudgetId{1}, Money(1000 * 100));
    b.spend(Money(300 * 100));
    CHECK(b.GetRemaining().GetKopecks() == 700 * 100);
    CHECK_THROWS(b.spend(Money(701 * 100)), BudgetExceeded);
    CHECK_THROWS(b.refund(Money(301 * 100)), RefundExceedsSpent);
    b.refund(Money(300 * 100));
    CHECK(b.GetSpent().GetKopecks() == 0);
}
 
int main() {
    test_remove_line_basic();
    test_remove_line_not_found();
    test_remove_line_only_in_draft();
    test_remove_last_line_then_submit();
    test_remove_line_frees_code_and_id();
    test_remove_line_through_service();
 
    test_submit_spends_budget();
    test_submit_refused_when_not_enough();
    test_submit_exactly_remaining();
    test_drafts_are_not_limited();
    test_cancel_refunds_budget();
    test_cancel_by_stranger_keeps_money_spent();
    test_failed_submit_does_not_spend();
    test_budget_aggregate();
 
    if (failed == 0) std::cout << "All tests completed without errors\n";
    else std::cout << "Attempts failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}