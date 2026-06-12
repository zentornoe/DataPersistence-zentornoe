#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct Sample {
    std::string id;                 // S-001
    std::string name;               // 시료명
    double      avgProductionTime;  // 평균 생산시간 (min/ea)
    double      yield;              // 수율 (0.0 ~ 1.0)
    int         stock;              // 실재고 (창고 보유 수량)
    int         reservedQty = 0;    // 예약 수량 (CONFIRMED 미출고분, 내부 관리용)

    // 실제 출고 가능 수량
    int availableStock() const { return stock - reservedQty; }
};

// to_json: reservedQty 포함하여 저장
inline void to_json(nlohmann::json& j, const Sample& s) {
    j = {
        { "id",                 s.id                 },
        { "name",               s.name               },
        { "avgProductionTime",  s.avgProductionTime  },
        { "yield",              s.yield              },
        { "stock",              s.stock              },
        { "reservedQty",        s.reservedQty        }
    };
}

// from_json: reservedQty 없는 기존 파일도 0으로 안전하게 읽음
inline void from_json(const nlohmann::json& j, Sample& s) {
    j.at("id").get_to(s.id);
    j.at("name").get_to(s.name);
    j.at("avgProductionTime").get_to(s.avgProductionTime);
    j.at("yield").get_to(s.yield);
    j.at("stock").get_to(s.stock);
    s.reservedQty = j.value("reservedQty", 0);
}
