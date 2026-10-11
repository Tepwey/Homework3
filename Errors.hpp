#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

struct DomainError : std::runtime_error {
    using std::runtime_error::runtime_error;
};


struct InvalidProductCode : DomainError {
    std::string code;
    explicit InvalidProductCode(std::string c)
        : DomainError("invalid product code: '" + c + "'"), code(std::move(c)) {}
};

struct InvalidProductPrice : DomainError {
    std::string code;
    std::int64_t kopecks;
    InvalidProductPrice(std::string c, std::int64_t k)
        : DomainError("invalid price for product " + c), code(std::move(c)), kopecks(k) {}
};

struct InvalidQuantity : DomainError {
    int count;
    explicit InvalidQuantity(int c)
        : DomainError("quantity must be in [1, 1000], got: " + std::to_string(c)), count(c) {}
};

struct MoneyOverflow : DomainError {
    std::int64_t kopecks;
    int factor;
    MoneyOverflow(std::int64_t k, int f)
        : DomainError("money overflow"), kopecks(k), factor(f) {}
};

struct RequestNotDraft : DomainError {
    std::int64_t request_id;
    explicit RequestNotDraft(std::int64_t id)
        : DomainError("request " + std::to_string(id) + " is not a draft"), request_id(id) {}
};

struct DuplicateLineId : DomainError {
    std::int64_t request_id;
    std::int64_t line_id;
    DuplicateLineId(std::int64_t r, std::int64_t l)
        : DomainError("duplicate line " + std::to_string(l) + " in request " + std::to_string(r)),
          request_id(r), line_id(l) {}
};

struct DuplicateProduct : DomainError {
    std::int64_t request_id;
    std::string code;
    DuplicateProduct(std::int64_t r, std::string c)
        : DomainError("duplicate product " + c + " in request " + std::to_string(r)),
          request_id(r), code(std::move(c)) {}
};

struct RequestLimitExceeded : DomainError {
    std::int64_t request_id, current_kopecks, additional_kopecks, limit_kopecks;
    RequestLimitExceeded(std::int64_t r, std::int64_t cur, std::int64_t add, std::int64_t lim)
        : DomainError("limit exceeded in request " + std::to_string(r)),
          request_id(r), current_kopecks(cur), additional_kopecks(add), limit_kopecks(lim) {}
};

struct EmptyRequest : DomainError {
    std::int64_t request_id;
    explicit EmptyRequest(std::int64_t r)
        : DomainError("request " + std::to_string(r) + " has no lines"), request_id(r) {}
};


struct LineNotFound : DomainError {
    std::int64_t request_id;
    std::int64_t line_id;
    LineNotFound(std::int64_t r, std::int64_t l)
        : DomainError("line " + std::to_string(l) + " not found in request " + std::to_string(r)),
          request_id(r), line_id(l) {}
};

struct CancellationDenied : DomainError {
    std::int64_t request_id, actor, author;
    CancellationDenied(std::int64_t r, std::int64_t a, std::int64_t au)
        : DomainError("only the author can cancel request " + std::to_string(r)),
          request_id(r), actor(a), author(au) {}
};

struct RequestNotSubmitted : DomainError {
    std::int64_t request_id;
    int status;
    RequestNotSubmitted(std::int64_t r, int s)
        : DomainError("request " + std::to_string(r) + " is not submitted"), request_id(r), status(s) {}
};

struct RequestChanged : DomainError {
    std::int64_t request_id, expected_kopecks, actual_kopecks;
    RequestChanged(std::int64_t r, std::int64_t expected, std::int64_t actual)
        : DomainError("request " + std::to_string(r) + " was changed during submission"),
          request_id(r), expected_kopecks(expected), actual_kopecks(actual) {}
};

struct InvalidBudgetLimit : DomainError {
    std::int64_t budget_id, kopecks;
    InvalidBudgetLimit(std::int64_t b, std::int64_t k)
        : DomainError("budget " + std::to_string(b) + " limit must not be negative"),
          budget_id(b), kopecks(k) {}
};

struct InvalidBudgetAmount : DomainError {
    std::int64_t budget_id, kopecks;
    InvalidBudgetAmount(std::int64_t b, std::int64_t k)
        : DomainError("amount must be positive for budget " + std::to_string(b)),
          budget_id(b), kopecks(k) {}
};

struct BudgetExceeded : DomainError {
    std::int64_t budget_id, remaining_kopecks, requested_kopecks;
    BudgetExceeded(std::int64_t b, std::int64_t remaining, std::int64_t requested)
        : DomainError("budget " + std::to_string(b) + " is exhausted"),
          budget_id(b), remaining_kopecks(remaining), requested_kopecks(requested) {}
};

struct RefundExceedsSpent : DomainError {
    std::int64_t budget_id, spent_kopecks, refund_kopecks;
    RefundExceedsSpent(std::int64_t b, std::int64_t spent, std::int64_t refund)
        : DomainError("refund exceeds spent amount of budget " + std::to_string(b)),
          budget_id(b), spent_kopecks(spent), refund_kopecks(refund) {}
};

struct BudgetNotFound : DomainError {
    std::int64_t budget_id;
    explicit BudgetNotFound(std::int64_t b)
        : DomainError("budget " + std::to_string(b) + " not found"), budget_id(b) {}
};

struct BudgetAlreadyExists : DomainError {
    std::int64_t budget_id;
    explicit BudgetAlreadyExists(std::int64_t b)
        : DomainError("budget " + std::to_string(b) + " already exists"), budget_id(b) {}
};


struct RequestNotFound : DomainError {
    std::int64_t request_id;
    explicit RequestNotFound(std::int64_t r)
        : DomainError("request " + std::to_string(r) + " not found"), request_id(r) {}
};

struct RequestAlreadyExists : DomainError {
    std::int64_t request_id;
    explicit RequestAlreadyExists(std::int64_t r)
        : DomainError("request " + std::to_string(r) + " already exists"), request_id(r) {}
};