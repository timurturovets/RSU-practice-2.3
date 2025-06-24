#include <vector>
#include "person.h"

class floor {
public:
    bool button_up = false;
    bool button_down = false;
    long long up_handler_id = -1;
    long long down_handler_id = -1;

    std::vector<person> persons = std::vector<person>();
    void switch_buttons_if_needed();
};