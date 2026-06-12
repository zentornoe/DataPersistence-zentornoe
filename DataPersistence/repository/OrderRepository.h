#pragma once
#include <vector>
#include <optional>
#include <string>
#include "../model/Order.h"

class OrderRepository {
public:
    explicit OrderRepository(const std::string& filePath);

    Order                   create(Order order);
    std::vector<Order>      findAll() const;
    std::optional<Order>    findById(const std::string& id) const;
    std::vector<Order>      findByStatus(OrderStatus status) const;
    std::vector<Order>      findBySampleId(const std::string& sampleId) const;
    bool                    update(const Order& order);
    bool                    remove(const std::string& id);

private:
    std::string       filePath_;
    std::vector<Order> load() const;
    void              save(const std::vector<Order>& orders) const;
    std::string       nextId() const;
    static std::string currentDateStr();
    static std::string currentDateTimeStr();
};
