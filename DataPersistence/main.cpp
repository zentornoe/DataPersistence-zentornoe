// DataPersistence PoC
// nlohmann/json 기반 JSON 파일 입출력 (CRUD) 데모
// 데이터 파일: data/samples.json, data/orders.json

#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"

// ---- 출력 헬퍼 ----
static void printSamples(const std::vector<Sample>& v) {
    if (v.empty()) { std::cout << "  (없음)\n"; return; }
    std::cout << "  ID      이름                    생산시간  수율   재고\n"
              << "  " << std::string(58, '-') << "\n";
    for (const auto& s : v)
        std::cout << "  " << std::left << std::setw(8) << s.id
                  << std::setw(24) << s.name
                  << std::setw(10) << s.avgProductionTime
                  << std::setw(7)  << s.yield
                  << s.stock << "\n";
}

static void printOrders(const std::vector<Order>& v) {
    if (v.empty()) { std::cout << "  (없음)\n"; return; }
    std::cout << "  주문번호                시료ID  고객명              수량  상태\n"
              << "  " << std::string(70, '-') << "\n";
    for (const auto& o : v)
        std::cout << "  " << std::left << std::setw(24) << o.id
                  << std::setw(8)  << o.sampleId
                  << std::setw(20) << o.customerName
                  << std::setw(6)  << o.quantity
                  << statusLabel(o.status) << "\n";
}

// ---- 시료 CRUD 메뉴 ----
static void sampleMenu(SampleRepository& repo) {
    while (true) {
        std::cout << "\n-- 시료 관리 ----------------------------------------\n"
                  << " 1.등록  2.전체조회  3.ID조회  4.이름검색  5.수정  6.삭제  0.뒤로\n"
                  << "선택: ";
        int c; std::cin >> c;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (c == 0) break;

        if (c == 1) {
            Sample s{};
            std::cout << "이름: ";          std::getline(std::cin, s.name);
            std::cout << "생산시간(min): "; std::cin >> s.avgProductionTime;
            std::cout << "수율(0~1): ";     std::cin >> s.yield;
            std::cout << "초기 재고: ";     std::cin >> s.stock;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            auto saved = repo.create(s);
            std::cout << "  → 저장 완료: " << saved.id << "\n";
        }
        else if (c == 2) {
            printSamples(repo.findAll());
        }
        else if (c == 3) {
            std::string id; std::cout << "ID: "; std::getline(std::cin, id);
            auto r = repo.findById(id);
            if (r) printSamples({*r});
            else   std::cout << "  없음\n";
        }
        else if (c == 4) {
            std::string kw; std::cout << "검색어: "; std::getline(std::cin, kw);
            printSamples(repo.findByName(kw));
        }
        else if (c == 5) {
            std::string id; std::cout << "수정할 ID: "; std::getline(std::cin, id);
            auto r = repo.findById(id);
            if (!r) { std::cout << "  없음\n"; continue; }
            Sample s = *r;
            std::cout << "새 재고(현재 " << s.stock << "ea): "; std::cin >> s.stock;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            repo.update(s);
            std::cout << "  → 수정 완료\n";
        }
        else if (c == 6) {
            std::string id; std::cout << "삭제할 ID: "; std::getline(std::cin, id);
            std::cout << "삭제하시겠습니까? (y/n): ";
            char yn; std::cin >> yn; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (yn == 'y' || yn == 'Y')
                std::cout << (repo.remove(id) ? "  → 삭제 완료\n" : "  없음\n");
        }
    }
}

// ---- 주문 CRUD 메뉴 ----
static void orderMenu(OrderRepository& oRepo, SampleRepository& sRepo) {
    while (true) {
        std::cout << "\n-- 주문 관리 ----------------------------------------\n"
                  << " 1.접수  2.전체조회  3.ID조회  4.상태변경  5.삭제  0.뒤로\n"
                  << "선택: ";
        int c; std::cin >> c;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (c == 0) break;

        if (c == 1) {
            printSamples(sRepo.findAll());
            Order o{};
            o.status = OrderStatus::RESERVED;
            std::cout << "시료 ID: ";   std::getline(std::cin, o.sampleId);
            if (!sRepo.findById(o.sampleId)) { std::cout << "  존재하지 않는 시료\n"; continue; }
            std::cout << "고객명: ";    std::getline(std::cin, o.customerName);
            std::cout << "수량(ea): ";  std::cin >> o.quantity;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            auto saved = oRepo.create(o);
            std::cout << "  → 접수 완료: " << saved.id << " [RESERVED]\n";
        }
        else if (c == 2) {
            printOrders(oRepo.findAll());
        }
        else if (c == 3) {
            std::string id; std::cout << "주문 ID: "; std::getline(std::cin, id);
            auto r = oRepo.findById(id);
            if (r) printOrders({*r});
            else   std::cout << "  없음\n";
        }
        else if (c == 4) {
            // 상태 변경: RESERVED → 승인/거절 / CONFIRMED → RELEASE
            std::cout << "처리할 주문 ID: "; std::string id; std::getline(std::cin, id);
            auto r = oRepo.findById(id);
            if (!r) { std::cout << "  없음\n"; continue; }
            Order o = *r;

            if (o.status == OrderStatus::RESERVED) {
                std::cout << "1.승인  2.거절: ";
                int sel; std::cin >> sel;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                if (sel == 1) {
                    auto sample = sRepo.findById(o.sampleId);
                    if (sample && sample->availableStock() >= o.quantity) {
                        // 재고 충분 → 예약 수량 증가, 실재고 유지
                        sample->reservedQty += o.quantity;
                        sRepo.update(*sample);
                        o.status = OrderStatus::CONFIRMED;
                        std::cout << "  → CONFIRMED (예약 수량 +" << o.quantity
                                  << ", 출고 가능 재고: " << sample->availableStock() << "ea)\n";
                    } else {
                        o.status = OrderStatus::PRODUCING;
                        std::cout << "  → PRODUCING (가용 재고 부족)\n";
                    }
                } else {
                    o.status = OrderStatus::REJECTED;
                    std::cout << "  → REJECTED\n";
                }
            } else if (o.status == OrderStatus::CONFIRMED) {
                // 출고 처리: 실재고 차감 + 예약 수량 해제
                auto sample = sRepo.findById(o.sampleId);
                if (sample) {
                    sample->stock      -= o.quantity;
                    sample->reservedQty -= o.quantity;
                    sRepo.update(*sample);
                }
                o.status = OrderStatus::RELEASE;
                std::cout << "  → RELEASE (실재고 -" << o.quantity
                          << "ea, 잔여 재고: " << (sample ? sample->stock : -1) << "ea)\n";
            } else {
                std::cout << "  이 상태(" << statusLabel(o.status) << ")는 변경 불가\n"; continue;
            }
            oRepo.update(o);
        }
        else if (c == 5) {
            std::string id; std::cout << "삭제할 주문 ID: "; std::getline(std::cin, id);
            std::cout << "삭제하시겠습니까? (y/n): ";
            char yn; std::cin >> yn; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (yn == 'y' || yn == 'Y')
                std::cout << (oRepo.remove(id) ? "  → 삭제 완료\n" : "  없음\n");
        }
    }
}

// ---- main ----
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    SampleRepository sampleRepo("data/samples.json");
    OrderRepository  orderRepo("data/orders.json");

    while (true) {
        auto samples = sampleRepo.findAll();
        auto orders  = orderRepo.findAll();
        int totalStock = 0; for (const auto& s : samples) totalStock += s.stock;

        std::cout << "\n====== S-Semi DataPersistence PoC ======\n"
                  << " 시료 " << samples.size() << "종 | 재고 " << totalStock
                  << "ea | 주문 " << orders.size() << "건\n"
                  << " [데이터 파일: data/samples.json, data/orders.json]\n"
                  << "----------------------------------------\n"
                  << " 1. 시료 관리 (Sample CRUD)\n"
                  << " 2. 주문 관리 (Order CRUD)\n"
                  << " 0. 종료\n"
                  << "선택: ";

        int choice; std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if      (choice == 1) sampleMenu(sampleRepo);
        else if (choice == 2) orderMenu(orderRepo, sampleRepo);
        else if (choice == 0) { std::cout << "종료\n"; break; }
    }
}
