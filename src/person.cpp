#include "../include/person.h"

person::person(int id, size_t weight, size_t enter_time, size_t floor_from, size_t floor_to) {
    this->id = id;
    this->weight = weight;
    this->enter_time = enter_time;
    this->floor_from = floor_from;
    this->floor_to = floor_to;

    elevator_enter_time = 0;
    time_moving = 0;
    was_overloading = false;
    people_met = std::vector<person>();
}

bool person::operator<(const person &other) const {
    return this->enter_time < other.enter_time;
}

bool person::operator==(const person &other) const {
    return this->id == other.id
        && this->weight == other.weight
        && this->enter_time == other.enter_time
        && this->floor_from == other.floor_from
        && this->floor_to == other.floor_to;
}