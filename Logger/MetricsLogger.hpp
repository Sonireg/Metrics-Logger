#pragma once
#include "Metrics/MetricsCollector.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

std::string getCurrentTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto in_time_t = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return ss.str();
}

class MetricsLogger : public MetricsCollector {
    std::thread loggingThread_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    bool isStopped_ = false;
    std::queue<std::vector<std::pair<std::string, std::string>>> metricsQueue_;
    std::ostream& logStream_;

    void loggingFunc() {
        while (true) {
            std::vector<std::pair<std::string, std::string>> metricsBatch;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                queueCondition_.wait(lock, [this] { return isStopped_ || !metricsQueue_.empty(); });
                if (isStopped_ && metricsQueue_.empty()) break;
                metricsBatch = std::move(metricsQueue_.front());
                metricsQueue_.pop();
            }
            if (!metricsBatch.empty()) {
                logStream_ << getCurrentTimestamp();
                for (const auto& [metricName, metricValue] : metricsBatch) {
                    logStream_ << " \"" << metricName << "\" " << metricValue;
                }
                logStream_ << std::endl;
            }
        }
    }

public:
    MetricsLogger(std::ostream& outputStream)
        : logStream_(outputStream)
    {
        loggingThread_ = std::thread(&MetricsLogger::loggingFunc, this);
    }

    ~MetricsLogger() {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            isStopped_ = true;
        }
        queueCondition_.notify_all();
        if (loggingThread_.joinable()) loggingThread_.join();
    }

    void requestFlush() {
        auto metricsBatch = snapshotAndReset();
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            metricsQueue_.push(std::move(metricsBatch));
        }
        queueCondition_.notify_one();
    }
};