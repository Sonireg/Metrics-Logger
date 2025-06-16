#include "Logger/MetricsLogger.hpp"
#include "Logger/PeriodicLogger.hpp"
#include <thread>
#include <chrono>
#include <string>

int main() {
    PeriodicLogger logger("metrics.log");
    logger.addMetric<double>("CPU", 0.95);
    logger.addMetric<int>("HTTP requests RPS", 0);
    logger.addMetric<std::string>("Status", "OK");
    logger.start(std::chrono::seconds(1)); // Start periodic logging

    logger.update<double>("CPU", 0.97);
    logger.update<int>("HTTP requests RPS", 42);
    logger.update<std::string>("Status", "OK");

    std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate runtime

    logger.stop(); // Stop periodic logging

    return 0;
}
