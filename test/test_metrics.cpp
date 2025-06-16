#include <gtest/gtest.h>
#include "../Logger/Metrics/MetricsCollector.hpp"
#include "../Logger/MetricsLogger.hpp"
#include "../Logger/PeriodicLogger.hpp"
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>

TEST(MetricsTest, AddAndUpdateMetric) {
    MetricsCollector collector;
    collector.addMetric<int>("HTTP RPS", 0);
    collector.update<int>("HTTP RPS", 42);

    auto snapshot = collector.snapshotAndReset();
    ASSERT_EQ(snapshot.size(), 1);
    EXPECT_EQ(snapshot[0].first, "HTTP RPS");
    EXPECT_EQ(snapshot[0].second, "42");
}

TEST(MetricsTest, ResetAfterSnapshot) {
    MetricsCollector collector;
    collector.addMetric<double>("CPU", 0.0);
    collector.update<double>("CPU", 0.75);
    collector.snapshotAndReset();
    auto snapshot2 = collector.snapshotAndReset();

    ASSERT_EQ(snapshot2.size(), 1);
    EXPECT_EQ(snapshot2[0].second, "0.000000");
}

TEST(MetricsTest, MetricsLoggerFlush) {
    std::ostringstream output;
    MetricsLogger logger(output);

    logger.addMetric<int>("A", 1);
    logger.update<int>("A", 100);

    logger.requestFlush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string log = output.str();
    EXPECT_TRUE(log.find("A") != std::string::npos);
    EXPECT_TRUE(log.find("100") != std::string::npos);
}

TEST(MetricsTest, PeriodicLoggerWorks) {
    std::ostringstream output;
    PeriodicLogger logger(output);
    logger.addMetric<int>("T", 5);
    logger.update<int>("T", 99);

    logger.start(std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    logger.stop();

    std::string log = output.str();
    EXPECT_TRUE(log.find("T") != std::string::npos);
    EXPECT_TRUE(log.find("99") != std::string::npos);
}

TEST(MetricsTest, ConcurrentUpdatesAreThreadSafe) {
    MetricsCollector collector;
    collector.addMetric<int>("Counter", 0);

    constexpr int threadCount = 10;
    constexpr int updatesPerThread = 100;

    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&collector, updatesPerThread]() {
            for (int i = 0; i < updatesPerThread; ++i) {
                collector.update<int>("Counter", i); 
            }
        });
    }

    for (auto& t : threads) t.join();

    auto snapshot = collector.snapshotAndReset();
    ASSERT_EQ(snapshot.size(), 1);
    EXPECT_EQ(snapshot[0].first, "Counter");
    int finalValue = std::stoi(snapshot[0].second);
    EXPECT_GE(finalValue, 0);
    EXPECT_LT(finalValue, updatesPerThread);
}

TEST(MetricsTest, MultipleFlushesLogged) {
    std::ostringstream output;
    MetricsLogger logger(output);
    logger.addMetric<int>("X", 1);

    logger.update<int>("X", 10);
    logger.requestFlush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    logger.update<int>("X", 20);
    logger.requestFlush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string log = output.str();
    size_t count = std::count(log.begin(), log.end(), '\n');
    EXPECT_EQ(count, 2); 
    EXPECT_TRUE(log.find("10") != std::string::npos);
    EXPECT_TRUE(log.find("20") != std::string::npos);
}

TEST(MetricsTest, UpdateWrongTypeThrows) {
    MetricsCollector collector;
    collector.addMetric<int>("IntMetric");

    EXPECT_THROW(collector.update<double>("IntMetric", 3.14), std::runtime_error);
}

TEST(MetricsTest, SnapshotReallyResets) {
    MetricsCollector collector;
    collector.addMetric<int>("M", 123);
    collector.update<int>("M", 456);

    auto snapshot1 = collector.snapshotAndReset();
    EXPECT_EQ(snapshot1[0].second, "456");

    collector.update<int>("M", 789);
    auto snapshot2 = collector.snapshotAndReset();
    EXPECT_EQ(snapshot2[0].second, "789");
}

TEST(MetricsTest, RemoveMetric) {
    MetricsCollector collector;
    collector.addMetric<int>("ToRemove", 1);
    collector.removeMetric("ToRemove");

    EXPECT_THROW(collector.update<int>("ToRemove", 5), std::runtime_error);
}
