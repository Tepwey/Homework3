#pragma once

#include <algorithm>
#include <vector>
#include "Errors.hpp"
#include "Values.hpp"

enum class RequestStatus { draft = 0, submitted = 1, cancelled = 2 };

class PurchaseRequest {
private:
    static constexpr std::int64_t limit_kopecks = 100000LL * 100;

    RequestId id;
    AuthorId author;
    RequestStatus status = RequestStatus::draft;
    std::vector<RequestLine> lines;

    void require_draft() const {
        if (status != RequestStatus::draft) throw RequestNotDraft(id.value);
    }

    std::vector<RequestLine>::const_iterator find(LineId line_id) const {
        return std::find_if(lines.begin(), lines.end(),
            [&](const RequestLine& l) { return l.GetId().value == line_id.value; });
    }

    bool contains(LineId line_id) const {
        return find(line_id) != lines.end();
    }
    
    bool has_product(const std::string& code) const {
        return std::any_of(lines.begin(), lines.end(),
            [&](const RequestLine& l) { return l.GetProduct().GetCode() == code; });
    }

    void ensure_addition_fits(std::int64_t current, std::int64_t additional) const {
        if (additional > limit_kopecks - current)
            throw RequestLimitExceeded(id.value, current, additional, limit_kopecks);
    }

public:
    PurchaseRequest(RequestId request_id, AuthorId request_author)
        : id(request_id), author(request_author) {}

    RequestId GetId() const {
        return id;
    }

    AuthorId GetAuthor() const {
        return author;
    }

    RequestStatus GetStatus() const {
        return status;
    }

    const std::vector<RequestLine>& GetLines() const {
        return lines;
    }

    std::int64_t total() const {
        std::int64_t sum = 0;
        for (const auto& l : lines) sum += l.count_amount().GetKopecks();
        return sum;
    }

    void add_line(LineId line_id, Product product, Quantity quantity) {
        require_draft();
        if (contains(line_id)) throw DuplicateLineId(id.value, line_id.value);
        if (has_product(product.GetCode())) throw DuplicateProduct(id.value, product.GetCode());
        RequestLine next(line_id, std::move(product), quantity);
        ensure_addition_fits(total(), next.count_amount().GetKopecks());
        lines.push_back(std::move(next));
    }

    void remove_line(LineId line_id) {
        require_draft();
        auto it = find(line_id);
        if (it == lines.end()) throw LineNotFound(id.value, line_id.value);
        lines.erase(it);
    }

    void submit() {
        require_draft();
        if (lines.empty()) throw EmptyRequest(id.value);
        status = RequestStatus::submitted;
    }

    void cancel(AuthorId actor) {
        if (actor.value != author.value)
            throw CancellationDenied(id.value, actor.value, author.value);
        if (status != RequestStatus::submitted)
            throw RequestNotSubmitted(id.value, static_cast<int>(status));
        status = RequestStatus::cancelled;
    }

};