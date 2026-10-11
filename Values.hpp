#pragma once

#include <cctype>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include "Errors.hpp"

struct RequestId { std::int64_t value; };
struct LineId    { std::int64_t value; };
struct AuthorId  { std::int64_t value; };

class Quantity {
private:
    int count;
public:
    explicit Quantity(int value) : count(value) {
        if (value < 1 || value > 1000) {
            throw InvalidQuantity(value);
        }
    }

    int GetCount() const { 
        return count;
    }
    bool operator==(const Quantity&) const = default;
};


class Money {
private:
    std::int64_t kopecks;

public:
    explicit Money(std::int64_t value) : kopecks(value) {}

    std::int64_t GetKopecks() const {
        return kopecks; 
    }

    Money operator*(Quantity q) const {
        if (kopecks > std::numeric_limits<std::int64_t>::max() / q.GetCount()) {
            throw MoneyOverflow(kopecks, q.GetCount());
        }
        return Money(kopecks * q.GetCount());
    }

    bool operator==(const Money&) const = default;
};


class Product {
private:
    std::string code;
    Money price;

    static bool valid_code_characters(const std::string& s) {
        for (unsigned char ch : s) {
            bool ok = std::isalnum(ch) || ch == '-' || ch == '_';
            if (!ok) return false;
        }
        return true;
    } 
public:
    Product(std::string product_code, Money product_price)
        : code(std::move(product_code)), price(product_price) {
            if (code.empty() || code.size() > 16 || !valid_code_characters(code)) {
                throw InvalidProductCode(code);
            }
            if (price.GetKopecks() <= 0) {
                throw InvalidProductPrice(code, price.GetKopecks());
            }
    }

    const std::string& GetCode() const {
        return code;
    }

    Money GetPrice() const {
        return price; 
    }
    
    bool operator==(const Product&) const = default;

};


class RequestLine {
private:
    LineId id;
    Product product;
    Quantity quantity;

public:
    RequestLine(LineId line_id, Product line_product, Quantity line_quantity)
        : id(line_id), product(std::move(line_product)), quantity(line_quantity) {}

    LineId GetId() const {
        return id;
    
    }
    const Product& GetProduct() const {
        return product;
    }
    
    Quantity GetQuantity() const {
        return quantity;
    }

    Money count_amount() const {
        return product.GetPrice() * quantity;
    }
};