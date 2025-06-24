#pragma once
#include <cstdlib>
#include "elevator.h"

class elevator;

class person {
public:
    int id;
    size_t weight;
    size_t enter_time;
    size_t floor_from;
    size_t floor_to;

    person() = default;
    person(int id, size_t weight, size_t enter_time, size_t floor_from, size_t floor_to);

    bool operator<(const person& other) const;
    bool operator==(const person &other) const;

    [[nodiscard]] bool is_moving_up() const { return floor_to > floor_from; }
};
