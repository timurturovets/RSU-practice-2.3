#pragma once

#include <cstdlib>
#include <vector>
#include <map>
#include <set>

#include "person.h"

class person;

class elevator {
public:
    enum state {
        moving_up,
        moving_down,
        doors_open,
        doors_closed
    };

    size_t id;
    size_t max_weight;
    state current_state;
    size_t current_floor;
    size_t floor_moving_to;
    size_t floor_moving_from;
    size_t moving_start_time;

    std::vector<person> persons_inside;
    std::map<size_t, bool> buttons = std::map<size_t, bool>();

    explicit elevator(size_t id, size_t max_weight, size_t floors_count);
    bool operator==(const elevator &other) const;

    [[nodiscard]] size_t get_current_load() const;
    [[nodiscard]] bool is_floor_travel_complete(size_t time_now) const;

    void add_person_inside(person &p, size_t time_now);

    size_t time_idling = 0;
    size_t time_moving = 0;
    size_t spans_between_floors_passed = 0;
    size_t max_load = 0;
    size_t overloads = 0;
    size_t load_sum = 0;
};