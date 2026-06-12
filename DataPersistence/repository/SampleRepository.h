#pragma once
#include <vector>
#include <optional>
#include <string>
#include "../model/Sample.h"

class SampleRepository {
public:
    explicit SampleRepository(const std::string& filePath);

    Sample                    create(Sample sample);
    std::vector<Sample>       findAll() const;
    std::optional<Sample>     findById(const std::string& id) const;
    std::vector<Sample>       findByName(const std::string& keyword) const;
    bool                      update(const Sample& sample);
    bool                      remove(const std::string& id);

private:
    std::string         filePath_;
    std::vector<Sample> load() const;
    void                save(const std::vector<Sample>& samples) const;
    std::string         nextId() const;
};
