#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

template <typename T>
class SafeValue {
   private:
    T value;
    T minValue;
    T maxValue;
    bool isValid;

   public:
    SafeValue(T val, T min, T max)
        : value(val),
          minValue(min),
          maxValue(max),
          isValid(val >= min && val <= max) {}

    bool isValidValue() const { return isValid; }

    T getValue() const {
        if (!isValid) {
            return minValue;
        }
        return value;
    }

    T getClampedValue() const {
        if (value < minValue) return minValue;
        if (value > maxValue) return maxValue;
        return value;
    }

    void setValue(T newValue) {
        value = newValue;
        isValid = (value >= minValue && value <= maxValue);
    }
};
