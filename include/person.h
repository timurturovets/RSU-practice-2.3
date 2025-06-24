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

    size_t elevator_enter_time;
    size_t time_moving;
    bool was_overloading;

    std::vector<person> people_met;

    person() = default;
    person(int id, size_t weight, size_t enter_time, size_t floor_from, size_t floor_to);
    person(person const &other) = default;

    bool operator<(const person& other) const;
    bool operator==(const person &other) const;

    [[nodiscard]] bool is_moving_up() const { return floor_to > floor_from; }
};
