#include "../include/floor.h"

void floor::switch_buttons_if_needed() {
    bool should_up_turn_off = true, should_down_turn_off = true;
    for (person const &p : persons) {
        if (p.is_moving_up()) should_up_turn_off = false;
        else should_down_turn_off = false;
    }

    if (should_up_turn_off) button_up = false;
    if (should_down_turn_off) button_down = false;
}