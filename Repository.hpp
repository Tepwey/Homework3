#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include "Budget.hpp"
#include "Errors.hpp"
#include "Purchase_request.hpp"
 

using RequestChange = std::function<void(PurchaseRequest&)>;
 
struct RequestUpdater {
    virtual void update(RequestId id, const RequestChange& change) = 0;
    virtual ~RequestUpdater() = default;
};
 
struct RequestLoader {
    virtual std::optional<PurchaseRequest> load(RequestId id) = 0;
    virtual ~RequestLoader() = default;
};
 
struct RequestCreator {
    virtual void create(const PurchaseRequest& request) = 0;
    virtual ~RequestCreator() = default;
};
 
struct PurchaseRequestRepository : RequestUpdater, RequestLoader, RequestCreator {};
 
class InMemoryPurchaseRequestRepository final : public PurchaseRequestRepository {
private:
    std::map<std::int64_t, PurchaseRequest> data;
    std::mutex mutex;
public:
    std::optional<PurchaseRequest> load(RequestId id) override {
        std::lock_guard<std::mutex> guard(mutex);
        auto it = data.find(id.value);
        if (it == data.end()) return std::nullopt;
        return it->second;
    }
 
    void create(const PurchaseRequest& request) override {
        std::lock_guard<std::mutex> guard(mutex);
        if (data.count(request.GetId().value))
            throw RequestAlreadyExists(request.GetId().value);
        data.emplace(request.GetId().value, request);
    }
 
    void update(RequestId id, const RequestChange& change) override {
        std::lock_guard<std::mutex> guard(mutex);
        auto it = data.find(id.value);
        if (it == data.end()) throw RequestNotFound(id.value);
        PurchaseRequest next = it->second;
        change(next);
        it->second = std::move(next);
    }
};


using BudgetChange = std::function<void(DepartmentBudget&)>;
 
struct BudgetUpdater {
    virtual void update(BudgetId id, const BudgetChange& change) = 0;
    virtual ~BudgetUpdater() = default;
};
 
struct BudgetLoader {
    virtual std::optional<DepartmentBudget> load(BudgetId id) = 0;
    virtual ~BudgetLoader() = default;
};
 
struct BudgetCreator {
    virtual void create(const DepartmentBudget& budget) = 0;
    virtual ~BudgetCreator() = default;
};
 
struct BudgetRepository : BudgetUpdater, BudgetLoader, BudgetCreator {};
 
class InMemoryBudgetRepository final : public BudgetRepository {
private:
    std::map<std::int64_t, DepartmentBudget> data;
    std::mutex mutex;
public:
    std::optional<DepartmentBudget> load(BudgetId id) override {
        std::lock_guard<std::mutex> guard(mutex);
        auto it = data.find(id.value);
        if (it == data.end()) return std::nullopt;
        return it->second;
    }
 
    void create(const DepartmentBudget& budget) override {
        std::lock_guard<std::mutex> guard(mutex);
        if (data.count(budget.GetId().value))
            throw BudgetAlreadyExists(budget.GetId().value);
        data.emplace(budget.GetId().value, budget);
    }
 
    void update(BudgetId id, const BudgetChange& change) override {
        std::lock_guard<std::mutex> guard(mutex);
        auto it = data.find(id.value);
        if (it == data.end()) throw BudgetNotFound(id.value);
        DepartmentBudget next = it->second;
        change(next);
        it->second = std::move(next);
    }
};