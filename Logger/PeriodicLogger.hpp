#pragma once
#include "MetricsLogger.hpp"
#include <thread>
#include <atomic>
#include <chrono>

class PeriodicLogger : public MetricsLogger {
    std::thread periodicThread_;
    std::atomic<bool> isRunning_{false};
    std::atomic<std::chrono::seconds> interval_{std::chrono::seconds(1)};

public:
    explicit PeriodicLogger(const std::string& filename) : MetricsLogger(filename) {}

    void start(std::chrono::seconds interval) {
        if (isRunning_) return;
        isRunning_ = true;
        this->interval_ = interval;
        periodicThread_ = std::thread([this]() {
            while (isRunning_) {
                std::this_thread::sleep_for(interval_.load());
                requestFlush();
            }
        });
    }

    void stop() {
        if (!isRunning_) return;
        isRunning_ = false;
        if (periodicThread_.joinable()) periodicThread_.join();
    }

    void updateInterval(std::chrono::seconds newInterval) {
        interval_ = newInterval;
    }

    ~PeriodicLogger() {
        stop();
    }
};