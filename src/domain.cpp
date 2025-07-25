#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include "../include/domain.h"

size_t domain::_floors_count = 0;
size_t domain::_persons_count = 0;
std::vector<elevator> domain::_elevators = std::vector<elevator>();
std::set<person> domain::_persons_unattached = std::set<person>();
std::map<size_t, floor> domain::_floors = std::map<size_t, floor>();
std::set<person> domain::_persons_data = std::set<person>();

void domain::run(const char * const * argv, int argc) {
    if (argv == nullptr) throw std::invalid_argument("no files paths have been provided.");
    if (argc < 3) throw std::invalid_argument("there must be at least 1 task for this program.");

    parse_config(argv[1]);

    parse_and_run_tasks(argv, argc);
}

void domain::inner_run() {
    size_t time_now = -1;
    while (true) {
        time_now++;

        if (_persons_unattached.empty()) { // loop end condition 1
            bool flag = true;
            for (auto const &pair : _floors) {
                if (!pair.second.persons.empty()) { // loop end condition 2
                    flag = false;
                    break;
                }
            }

            for (elevator const &e : _elevators) {
                if (e.current_state != elevator::doors_closed || !e.persons_inside.empty()) { // loop end condition 3
                    flag = false;
                }
            }

            if (flag) return;
        }

        for (auto it = _persons_unattached.begin(); it != _persons_unattached.end(); ) {
            auto p = *it;
            if (p.enter_time != time_now) {
                it++;
                continue;
            }

            if (p.is_moving_up()) _floors[p.floor_from].button_up = true;
            else _floors[p.floor_from].button_down = true;

            _floors[p.floor_from].persons.push_back(p);
            it = _persons_unattached.erase(it);

        }

        for (auto const &pair : _floors) { // каждому этажу устанавливаем ближайший к нему лифт на данный момент
            size_t closest_elevator_floor = _elevators[0].current_floor;
            elevator *closest_elevator = &_elevators[0];

            for (elevator &e : _elevators) {
                size_t val = which_number_is_closer(closest_elevator_floor, e.current_floor, pair.first);

                if (val == e.current_floor) {
                    closest_elevator_floor = e.current_floor;
                    closest_elevator = &e;
                }
            }

            _floors[pair.first].closest_elevator = closest_elevator;
        }

        for (elevator &e : _elevators) {
            switch(e.current_state) {
                case elevator::doors_open: {// если двери открыты и он готов впускать / выпускать людей
                    e.buttons[e.current_floor] = false;

                    for (auto it = e.persons_inside.begin(); it != e.persons_inside.end();) { // сначала выпускаем
                        person pe = *it;
                        if (pe.floor_to == e.current_floor) {
                            _persons_data.insert(person(pe));

                            it = e.persons_inside.erase(it);
                            continue;
                        }
                        it++;
                    }

                    // впускаем
                    for (auto it = _floors[e.current_floor].persons.begin(); it != _floors[e.current_floor].persons.end(); ) {
                        person &p = *it;

                        // если текущий человек войдёт, и будет перевес, сразу смотрим следующего
                        if (e.get_current_load() + p.weight > e.max_weight) {
                            p.was_overloading = true;
                            e.overloads++;

                            it++;
                            continue;
                        }

                        if (e.persons_inside.empty()) {
                            // в лифте никого нет, просто закидываем челика внутрь
                            e.add_person_inside(p, time_now);

                            e.buttons[p.floor_to] = true;
                            it = _floors[e.current_floor].persons.erase(it);
                            _floors[e.current_floor].switch_buttons_if_needed();
                            continue;
                        } else { // в лифте уже кто-то есть
                            // у всех людей уже в лифте одинаковое направление движения, просто смотрим первого
                            if (e.persons_inside[0].floor_to > e.persons_inside[0].floor_from) {
                            // если направление движения - вверх, заходят только люди, едущие вверх
                                if (p.is_moving_up()) {
                                    e.add_person_inside(p, time_now); // заталкиваем его
                                    e.buttons[p.floor_to] = true;

                                    it = _floors[e.current_floor].persons.erase(it); // удаляем с этажа
                                    _floors[e.current_floor].switch_buttons_if_needed();
                                    continue;
                                }

                                it++;
                            } else {
                                if (!p.is_moving_up()) {
                                    e.add_person_inside(p, time_now);
                                    e.buttons[p.floor_to] = true;

                                    it = _floors[e.current_floor].persons.erase(it);
                                    _floors[e.current_floor].switch_buttons_if_needed();
                                } else {
                                    it++;
                                    continue;
                                }
                            }
                        }
                    }
                    size_t load_now = e.get_current_load();
                    if (load_now > e.max_load) e.max_load = load_now;

                    // на этом моменте все, кто должен был, уже вышли и зашли

                    // смотрим, есть ли на этаже ещё люди, которым надо вверх/вниз, если надо - переключаем кнопки
                    _floors[e.current_floor].switch_buttons_if_needed();

                    if (_floors[e.current_floor].up_handler_id == e.id) _floors[e.current_floor].up_handler_id = -1;
                    if (_floors[e.current_floor].down_handler_id == e.id) _floors[e.current_floor].down_handler_id = -1;

                    if (!e.persons_inside.empty()) {
                        if (e.persons_inside[0].is_moving_up()) e.current_state = elevator::moving_up;
                        else e.current_state = elevator::moving_down;

                        size_t next_floor = e.persons_inside[0].floor_to;
                        for (auto pair: e.buttons) {
                            if (pair.second
                                && which_number_is_closer(pair.first, next_floor, e.current_floor) == pair.first) {
                                next_floor = pair.first;
                            }
                        }

                        e.floor_moving_to = next_floor;
                        break;
                    }

                    e.current_state = elevator::doors_closed;
                    e.moving_start_time = 0;
                    break;
                }

                case elevator::doors_closed: // если двери закрыты - либо пора ехать, либо лифт ждёт вызова
                {
                    e.time_idling++;
                    size_t floor_demanded_inside = 0;
                    // находим ближайший этаж из тех, что запрашиваемы внутри
                    for (auto pair : e.buttons) {
                        if (pair.second
                        && which_number_is_closer(pair.first, floor_demanded_inside, e.current_floor) == pair.first) {
                            floor_demanded_inside = pair.first;
                        }
                    }

                    size_t mff_number = 0;
                    bool mff_handling_direction = false; // false - вниз, true - вверх
                    for (auto const &pair : _floors) {
                        if (pair.second.button_up) { // сначала самый подходящий этаж - просто первый с нажатой кнопкой
                            if (pair.second.up_handler_id == -1) { // этажу не назначен лифт едущий вверх
                                mff_number = pair.first;
                                mff_handling_direction = true;
                            }
                            break;
                        }

                        if (pair.second.button_down) {
                            if (pair.second.down_handler_id == -1) { // этажу не назначен лифт едущий вниз
                                mff_number = pair.first;
                                mff_handling_direction = false;
                            }
                        }
                    }


                    if (mff_number == 0) { // ни на одном этаже пока не нажаты кнопки
                        if (floor_demanded_inside != 0) e.floor_moving_to = floor_demanded_inside;
                        continue; // едем туда, куда просят люди внутри, если такой запрос вообще есть, иначе просто дальше
                    }

                    // затем самый подходящий этаж - этаж с людьми и ближе, чем найденный в тупую
                    for (auto const &pair: _floors) {
                        // проходим по этажам, чтобы найти самый подходящий - самый близкий и на котором есть люди
                        if (which_number_is_closer(pair.first, mff_number, e.current_floor) == pair.first) {
                            // этаж нашли, однако возможно уже есть лифт, который выполняет задачу на этот этаж - проверко
                            if (pair.second.button_up) {
                                if (pair.second.up_handler_id == -1) { // если на этом этаже лифта на задании вверх нет
                                    mff_number = pair.first;
                                    mff_handling_direction = true;
                                    continue;
                                }
                            }

                            if (pair.second.button_down) {
                                if (pair.second.down_handler_id == -1) {
                                    mff_number = pair.first;
                                    mff_handling_direction = false;
                                }
                            }
                        }
                    }

                    if (_floors[mff_number].closest_elevator != nullptr && _floors[mff_number].closest_elevator != &e) {
                        continue; // есть лифт ближе, он и должен приехать
                    }

                    e.current_state = mff_number > e.current_floor
                            ? elevator::moving_up
                            : elevator::moving_down;

                    if (mff_handling_direction) _floors[mff_number].up_handler_id = e.id;
                    else _floors[mff_number].down_handler_id = e.id;

                    if (e.floor_moving_from < 1) e.floor_moving_from = e.current_floor;
                    e.floor_moving_to = mff_number;
                    e.moving_start_time = time_now;

                    break;
                }

                default: // elevator::moving_up || elevator::moving_down
                {
                    handle_traveling(e, time_now);
                    break;
                }
            }
        }
    }
}

void domain::handle_traveling(elevator &e, size_t time_now) {
    if (e.current_state != elevator::state::moving_up && e.current_state != elevator::state::moving_down) {
        throw std::invalid_argument("state must be equal to moving_up or moving_down");
    }

    e.time_moving++;

    for (person &p : e.persons_inside) {
        p.time_moving++;
    }

    if (e.is_floor_travel_complete(time_now)) {
        // если лифт проехал расстояние от прошлого этажа до очередного
        if (e.current_state == elevator::moving_up) e.current_floor++;
        else e.current_floor--;

        e.spans_between_floors_passed++;
        e.moving_start_time = time_now;

        bool button_condition = e.current_state == elevator::moving_up
                ? _floors[e.current_floor].button_up
                : _floors[e.current_floor].button_down;

        if (e.current_floor == e.floor_moving_to || button_condition) { // лифт доехал куда надо
            e.current_state = elevator::doors_open;
            e.buttons[e.current_floor] = false;
            e.floor_moving_to = 0;

            return;
        }

    }

    size_t closest_demanded_floor_number = e.floor_moving_to;
    if (closest_demanded_floor_number == 0) { // этаж движения ещё не задан
        if (!e.persons_inside.empty()) {
            size_t closest_demanded_floor_from_people_inside = e.persons_inside[0].floor_to;
            for (person const &p: e.persons_inside) {
                if (which_number_is_closer(closest_demanded_floor_from_people_inside, p.floor_to, e.current_floor) ==
                    p.floor_to) {
                    closest_demanded_floor_from_people_inside = p.floor_to;
                }
            }
        }
    }

    for (auto const &pair : _floors) {
        // смотрим - есть ли этажи раньше целевого, на котором есть запрос
        if (e.current_state == elevator::moving_up) {
            if (pair.first <= e.current_floor || pair.first > closest_demanded_floor_number) continue;

            if (pair.second.button_up) closest_demanded_floor_number = pair.first;
        } else {
            if (pair.first >= e.current_floor || pair.first < closest_demanded_floor_number) continue;

            if (pair.second.button_down) closest_demanded_floor_number = pair.first;
        }
    }

    e.floor_moving_to = closest_demanded_floor_number;
}
void domain::parse_config(const char *config_file_path) {
    std::ifstream file_stream(config_file_path);

    if (!file_stream.is_open()) throw std::ifstream::failure("file could not be opened.");

    std::string line;

    getline(file_stream, line);
    size_t n = std::stoull(line);

    getline(file_stream, line);
    size_t k = std::stoull(line);

    _floors_count = n;

    for (size_t i = 0; i < n; i++) {
        _floors[i + 1] = floor{false, false, -1, -1, nullptr,
                               std::vector<person>()};
    }

    for (size_t i = 0; i < k; i++) {
        getline(file_stream, line);
        _elevators.emplace_back(i + 1, std::stoull(line), _floors_count);
    }
}

void domain::parse_and_run_tasks(const char * const * tasks_files_paths, size_t tasks_len) {
    std::ofstream elevators_file("D:\\1elevator\\elevators_data.txt");
    std::ofstream persons_file("D:\\1elevator\\persons_data.txt");

    if (!elevators_file.is_open() || !persons_file.is_open()) {
        throw std::ofstream::failure("output files could not be opened or created");
    }

    for (size_t i = 2; i < tasks_len; i++) {
        std::ifstream file_stream(*(tasks_files_paths + i));
        std::string line, id, weight, floor_from, time, floor_to;

        _persons_unattached.clear();
        _persons_count = 0;

        while (getline(file_stream, line)) {
            std::istringstream iss(line);
            iss >> id >> weight >> floor_from >> time >> floor_to;

            size_t s_time = std::stoull(time.substr(0, 2)) * 60 + std::stoull(time.substr(3, 2));
            _persons_unattached.insert(person(
                    std::stoi(id),
                    std::stoull(weight),
                    s_time,
                    std::stoull(floor_from),
                    std::stoull(floor_to)
                    )
                );

            _persons_count++;
        }

        inner_run();

        for (person const &p : _persons_data) {
            persons_file << "Person ID: " << p.id << std::endl;
            persons_file << "Entered the elevator at: " << p.elevator_enter_time << std::endl;
            persons_file << "How much time was moving: " << p.time_moving << std::endl;
            persons_file << "Caused overloads: " << (p.was_overloading ? "yes" : "no") << std::endl;
            persons_file << "IDs of people they've met: " << (p.people_met.empty() ? "they didn't meet any" : "");
            for (person const &pm : p.people_met) {
                persons_file << pm.id << " ";
            }

            persons_file << std::endl << std::endl;
        }
    }

    for (elevator const &e : _elevators) {
        elevators_file << "Elevator with ID: " << e.id << std::endl;
        elevators_file << "Time idling: " << e.time_idling << std::endl;
        elevators_file << "Time moving: " << e.time_moving << std::endl;
        elevators_file << "Spans between floors passed: " << e.spans_between_floors_passed << std::endl;
        elevators_file << "Load summed up: " << e.load_sum << std::endl;
        elevators_file << "Maximum load: " << e.max_load << std::endl;
        elevators_file << "Overloads: " << e.overloads << std::endl << std::endl;
    }
}

size_t domain::which_number_is_closer(size_t a, size_t b, size_t closer_to) {
    std::size_t dist_a = (a >= closer_to) ? (a - closer_to) : (closer_to - a);
    std::size_t dist_b = (b >= closer_to) ? (b - closer_to) : (closer_to - b);

    if (dist_a < dist_b) return a;
    if (dist_b < dist_a) return b;

    return closer_to;
}