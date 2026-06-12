#include "OrderRepository.h"
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <cstdio>

using json = nlohmann::json;
namespace fs = std::filesystem;

OrderRepository::OrderRepository(const std::string& filePath)
    : filePath_(filePath)
{
    auto dir = fs::path(filePath).parent_path();
    if (!dir.empty()) fs::create_directories(dir);
    if (!fs::exists(filePath_)) save({});
}

std::vector<Order> OrderRepository::load() const {
    std::ifstream f(filePath_);
    if (!f.is_open()) return {};
    json j = json::parse(f, nullptr, /*exceptions=*/false);
    if (j.is_discarded() || !j.contains("orders")) return {};
    return j["orders"].get<std::vector<Order>>();
}

void OrderRepository::save(const std::vector<Order>& orders) const {
    std::ofstream f(filePath_);
    json j;
    j["orders"] = orders;
    f << j.dump(2);
}

std::string OrderRepository::currentDateStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &t);
    char buf[16];
    snprintf(buf, sizeof(buf), "%04d%02d%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    return buf;
}

std::string OrderRepository::currentDateTimeStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &t);
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    return buf;
}

std::string OrderRepository::nextId() const {
    std::string date   = currentDateStr();
    std::string prefix = "ORD-" + date + "-";
    int maxSeq = 0;
    for (const auto& o : load()) {
        if (o.id.rfind(prefix, 0) == 0) {
            try { maxSeq = std::max(maxSeq, std::stoi(o.id.substr(prefix.size()))); }
            catch (...) {}
        }
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "ORD-%s-%04d", date.c_str(), maxSeq + 1);
    return buf;
}

Order OrderRepository::create(Order order) {
    if (order.id.empty())        order.id        = nextId();
    if (order.createdAt.empty()) order.createdAt = currentDateTimeStr();
    auto all = load();
    all.push_back(order);
    save(all);
    return order;
}

std::vector<Order> OrderRepository::findAll() const {
    return load();
}

std::optional<Order> OrderRepository::findById(const std::string& id) const {
    for (const auto& o : load())
        if (o.id == id) return o;
    return std::nullopt;
}

std::vector<Order> OrderRepository::findByStatus(OrderStatus status) const {
    std::vector<Order> result;
    for (const auto& o : load())
        if (o.status == status) result.push_back(o);
    return result;
}

std::vector<Order> OrderRepository::findBySampleId(const std::string& sampleId) const {
    std::vector<Order> result;
    for (const auto& o : load())
        if (o.sampleId == sampleId) result.push_back(o);
    return result;
}

bool OrderRepository::update(const Order& order) {
    auto all = load();
    for (auto& o : all) {
        if (o.id == order.id) { o = order; save(all); return true; }
    }
    return false;
}

bool OrderRepository::remove(const std::string& id) {
    auto all = load();
    auto it = std::remove_if(all.begin(), all.end(),
        [&](const Order& o) { return o.id == id; });
    if (it == all.end()) return false;
    all.erase(it, all.end());
    save(all);
    return true;
}
