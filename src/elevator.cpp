#include "../include/elevator.h"
#include <cstdlib>
#include <cmath>

elevator::elevator(size_t id, size_t max_weight, size_t floors_count) : persons_inside(std::vector<person>()) {
    this->id = id;
    this->max_weight = max_weight;

    current_state = elevator::state::doors_closed;
    current_floor = 1;

    moving_start_time = -1;
    floor_moving_to = -1;
    floor_moving_from = -1;
    stub = std::vector<int>();

    for (size_t i = 0; i < floors_count; i++) {
        buttons[i + 1] = false;
    }
}

size_t elevator::get_current_load() const {
    size_t sum = 0;

    for (const person &p : persons_inside) {
        sum += p.weight;
    }

    return sum;
}

bool elevator::is_floor_travel_complete(size_t time_now) const {
    return time_now >= moving_start_time
        + (3 + 5 * ((size_t)std::ceil(get_current_load() / max_weight)));
}

bool elevator::operator==(const elevator &other) const {
    return this->id == other.id;
}