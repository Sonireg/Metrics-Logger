#pragma once
#include <string>

class IMetric {
public:
    virtual ~IMetric() = default;
    virtual std::string getName() const = 0;
    virtual std::string toString() const = 0;
    virtual void reset() = 0;
};