#include <cstdlib>
#include <vector>
#include <set>
#include <map>

#include "elevator.h"
#include "floor.h"

class domain {
public:
    static void run(const char * const * argv, int argc);

private:

    static size_t _tasks_len;
    static size_t _elevators_count;
    static size_t _floors_count;
    static size_t _persons_count;

    static std::vector<elevator> _elevators;
    static std::set<person> _persons_unattached;
    static std::map<size_t, floor> _floors;

    static void parse_config(char const *config_file_path);
    static void parse_and_run_tasks(char const *const *tasks_files_paths, size_t tasks_len);
    static void inner_run();

    static void handle_traveling(elevator &e, size_t time_now);
    static size_t which_number_is_closer(size_t a, size_t b, size_t closer_to);
};