#include "SampleRepository.h"
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <cstdio>

using json = nlohmann::json;
namespace fs = std::filesystem;

SampleRepository::SampleRepository(const std::string& filePath)
    : filePath_(filePath)
{
    auto dir = fs::path(filePath).parent_path();
    if (!dir.empty()) fs::create_directories(dir);
    if (!fs::exists(filePath_)) save({});
}

std::vector<Sample> SampleRepository::load() const {
    std::ifstream f(filePath_);
    if (!f.is_open()) return {};
    json j = json::parse(f, nullptr, /*exceptions=*/false);
    if (j.is_discarded() || !j.contains("samples")) return {};
    return j["samples"].get<std::vector<Sample>>();
}

void SampleRepository::save(const std::vector<Sample>& samples) const {
    std::ofstream f(filePath_);
    json j;
    j["samples"] = samples;
    f << j.dump(2);
}

std::string SampleRepository::nextId() const {
    int maxNum = 0;
    for (const auto& s : load()) {
        if (s.id.size() > 2 && s.id[0] == 'S' && s.id[1] == '-') {
            try { maxNum = std::max(maxNum, std::stoi(s.id.substr(2))); }
            catch (...) {}
        }
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "S-%03d", maxNum + 1);
    return buf;
}

Sample SampleRepository::create(Sample sample) {
    if (sample.id.empty()) sample.id = nextId();
    auto all = load();
    all.push_back(sample);
    save(all);
    return sample;
}

std::vector<Sample> SampleRepository::findAll() const {
    return load();
}

std::optional<Sample> SampleRepository::findById(const std::string& id) const {
    for (const auto& s : load())
        if (s.id == id) return s;
    return std::nullopt;
}

std::vector<Sample> SampleRepository::findByName(const std::string& keyword) const {
    std::vector<Sample> result;
    for (const auto& s : load())
        if (s.name.find(keyword) != std::string::npos) result.push_back(s);
    return result;
}

bool SampleRepository::update(const Sample& sample) {
    auto all = load();
    for (auto& s : all) {
        if (s.id == sample.id) { s = sample; save(all); return true; }
    }
    return false;
}

bool SampleRepository::remove(const std::string& id) {
    auto all = load();
    auto it = std::remove_if(all.begin(), all.end(),
        [&](const Sample& s) { return s.id == id; });
    if (it == all.end()) return false;
    all.erase(it, all.end());
    save(all);
    return true;
}
