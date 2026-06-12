#pragma once
#include <string>
#include <nlohmann/json.hpp>

enum class OrderStatus { RESERVED, CONFIRMED, PRODUCING, RELEASE, REJECTED };

NLOHMANN_JSON_SERIALIZE_ENUM(OrderStatus, {
    { OrderStatus::RESERVED,  "RESERVED"  },
    { OrderStatus::CONFIRMED, "CONFIRMED" },
    { OrderStatus::PRODUCING, "PRODUCING" },
    { OrderStatus::RELEASE,   "RELEASE"   },
    { OrderStatus::REJECTED,  "REJECTED"  },
})

inline std::string statusLabel(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::RELEASE:   return "RELEASE";
    case OrderStatus::REJECTED:  return "REJECTED";
    }
    return "UNKNOWN";
}

struct Order {
    std::string id;           // ORD-YYYYMMDD-NNNN
    std::string sampleId;     // S-001
    std::string customerName; // 고객명
    int         quantity;     // 주문 수량
    OrderStatus status;       // 주문 상태
    std::string createdAt;    // 생성일시 (ISO)
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Order, id, sampleId, customerName, quantity, status, createdAt)
