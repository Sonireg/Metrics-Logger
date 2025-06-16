# Metrics Logger Library

Библиотека для сбора и логирования метрик в многопоточной среде. Поддерживает два типа логирования:
- ручное логирование (`MetricsLogger`)
- периодическое логирование (`PeriodicLogger`)

## Структура проекта
```
Logger/
├── Metrics/
    ├── IMetric.hpp # Интерфейс метрики
    ├── Metric.hpp # Шаблонный класс метрики
    ├── MetricsCollector.hpp # Коллектор метрик
├── MetricsLogger.hpp # Логгер с ручным логированием
└── PeriodicLogger.hpp # Периодический логгер
tests/
└── MetricsTests.cpp # GTest-тесты
main.cpp # Пример использования логеров
```
## Классы и интерфейсы

### `IMetric`

Абстрактный интерфейс метрики:
- `std::string getName() const`: имя метрики
- `std::string toString() const`: значение метрики в виде строки
- `void reset()`: сброс значения метрики

### `template <typename T> class Metric`

Реализация метрики для произвольного типа `T` (например, `int`, `double`, `std::string`):
- `get() const`: получить текущее значение
- `update(const T&)`: обновить значение
- `toString()`: строковое представление
- `reset()`: сброс к значению по умолчанию

### `MetricsCollector`

Контейнер и менеджер всех метрик:
- `addMetric<T>(name, defaultValue)`: добавить метрику
- `update<T>(name, value)`: обновить метрику
- `snapshotAndReset()`: вернуть текущие значения всех метрик и сбросить их
- `removeMetric(name)`, `clearMetrics()`: управление метриками

### `MetricsLogger : public MetricsCollector`

Логгер, записывающий метрики в фоновом потоке по запросу:
- `requestFlush()`: делает `snapshotAndReset()` и пишет в поток
- Логирует в формате:
2025-06-16 12:00:01.123 "metric_name" value ...

### `PeriodicLogger : public MetricsLogger`

Запускает фоновый поток, который периодически вызывает `requestFlush()`:
- `start(interval)`: запускает логирование каждые `interval` секунд
- `stop()`: останавливает логгирование
- `updateInterval(newInterval)`: изменить интервал

## Пример использования

```cpp
#include "Logger/PeriodicLogger.hpp"
#include <fstream>
#include <thread>

int main() {
  std::ofstream logFile("metrics.log");
  PeriodicLogger logger(logFile);

  logger.addMetric<int>("RPS", 0);
  logger.start(std::chrono::seconds(1));

  for (int i = 0; i < 5; ++i) {
      logger.update<int>("RPS", i * 10);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  logger.stop();
}
```
## Сборка и запуск тестов
Тесты написаны с использованием Google Test. Убедитесь, что он установлен:

Установка Google Test (Ubuntu)
```bash
sudo apt install libgtest-dev
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib *.a /usr/lib
```
Сборка и запуск
```bash
g++ -std=c++20 tests/MetricsTests.cpp -o tests_run -lgtest -lpthread
./tests_run
```
## Сборка и запуск main
```bash
g++ -std=c++20 main.cpp -o logger_demo -pthread
./logger_demo
```
Логи будут записаны в standard_logger.log и periodic_logger.log.

## Примечания
Все операции потокобезопасны

Используется std::mutex и std::thread

Поддержка шаблонных метрик с автоопределением типа

Возможность логирования в stdout, ofstream или любой другой std::ostream