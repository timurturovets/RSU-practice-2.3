cmake_minimum_required(VERSION 3.25)
project(solutions2_3)

set(CMAKE_CXX_STANDARD 17)

add_executable(solutions2_3 main.cpp include/domain.h src/domain.cpp include/person.h include/elevator.h src/elevator.cpp src/person.cpp include/floor.h src/floor.cpp)
