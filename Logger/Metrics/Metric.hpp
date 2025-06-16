#pragma once
#include "IMetric.hpp"
#include <mutex>
#include <string>
#include <type_traits>

template<typename T>
class Metric : public IMetric {
    std::string name_;
    T value_;
    T defaultValue_;
    mutable std::mutex mtx_;
public:
    Metric(const std::string& name, const T& defaultValue = T{}) 
        : name_(name), value_(defaultValue), defaultValue_(defaultValue) {}

    std::string getName() const override {
        return name_;
    }

    T get() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return value_;
    }

    void update(const T& val) {
        std::lock_guard<std::mutex> lock(mtx_);
        value_ = val;
    }

    std::string toString() const override {
        std::lock_guard lock(mtx_);
        if constexpr (std::is_same_v<T, std::string>) {
            return "\"" + value_ + "\"";
        } else {
            return std::to_string(value_);
        }
    }

    void reset() override {
        std::lock_guard<std::mutex> lock(mtx_);
        value_ = defaultValue_;
    }
};