#include "Logger/MetricsLogger.hpp"
#include "Logger/PeriodicLogger.hpp"
#include <fstream>
#include <thread>
#include <random>
#include <atomic>
#include <chrono>
#include <iostream>

int getRandomValue(int min, int max) {
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

void simulateRequests(MetricsLogger& logger, std::atomic<bool>& stopFlag) {
    while (!stopFlag.load()) {
        try {
            logger.update<int>("HTTP RPS", getRandomValue(10, 100));
            logger.update<std::string>("LastEndpoint", "/api/v1/resource");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            logger.requestFlush();
        } catch (const std::exception& ex) {
            std::cerr << "Logger Error: " << ex.what() << std::endl;
        }
    }
}

void simulateCPU(PeriodicLogger& logger, std::atomic<bool>& stopFlag) {
    while (!stopFlag.load()) {
        try {
            logger.update<double>("CPU Usage", getRandomValue(10, 90) / 100.0);
            logger.update<int>("Threads", getRandomValue(1, 8));
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        } catch (const std::exception& ex) {
            std::cerr << "PeriodicLogger Error: " << ex.what() << std::endl;
        }
    }
}

int main() {
    std::ofstream logFile1("standard_logger.log");
    std::ofstream logFile2("periodic_logger.log");

    if (!logFile1 || !logFile2) {
        std::cerr << "Failed to open log files." << std::endl;
        return 1;
    }

    MetricsLogger standardLogger(logFile1);
    PeriodicLogger periodicLogger(logFile2);

    standardLogger.addMetric<int>("HTTP RPS", 0);
    standardLogger.addMetric<std::string>("LastEndpoint", "");

    periodicLogger.addMetric<double>("CPU Usage", 0.0);
    periodicLogger.addMetric<int>("Threads", 1);

    periodicLogger.start(std::chrono::seconds(1));

    std::atomic<bool> stopFlag{false};

    std::thread t1(simulateRequests, std::ref(standardLogger), std::ref(stopFlag));
    std::thread t2(simulateCPU, std::ref(periodicLogger), std::ref(stopFlag));

    std::this_thread::sleep_for(std::chrono::seconds(5));
    stopFlag = true;

    t1.join();
    t2.join();

    periodicLogger.stop();

    std::cout << "Logging finished. Check 'standard_logger.log' and 'periodic_logger.log'." << std::endl;
    return 0;
}
