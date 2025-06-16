#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include "IMetric.hpp"
#include "Metric.hpp"

class MetricsCollector {
    std::unordered_map<std::string, std::shared_ptr<IMetric>> metrics_;
    mutable std::mutex mtx_;
public:
    template<typename T>
    void addMetric(const std::string& name, const T& defaultValue = T{}) {
        std::lock_guard lock(mtx_);
        if (metrics_.find(name) == metrics_.end()) {
            metrics_[name] = std::make_shared<Metric<T>>(name, defaultValue);
        }
    }

    template<typename T>
    void update(const std::string& name, const T& value) {
        std::lock_guard lock(mtx_);
        auto it = metrics_.find(name);
        if (it != metrics_.end()) {
            auto ptr = std::dynamic_pointer_cast<Metric<T>>(it->second);
            if (ptr) {
                ptr->update(value);
            } else {
                throw std::runtime_error("Metric type mismatch for " + name);
            }
        } else {
            throw std::runtime_error("Metric not found: " + name);
        }
    }

    std::vector<std::pair<std::string, std::string>> snapshotAndReset() {
        std::lock_guard<std::mutex> lock(mtx_);
        std::vector<std::pair<std::string, std::string>> res;
        for (auto& [name, metric] : metrics_) {
            res.emplace_back(name, metric->toString());
            metric->reset();
        }
        return res;
    }

    void removeMetric(const std::string& name) {
        std::lock_guard<std::mutex> lock(mtx_);
        metrics_.erase(name);
    }

    void clearMetrics() {
        std::lock_guard<std::mutex> lock(mtx_);
        metrics_.clear();
    }
};