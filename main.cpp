#include <iostream>
#include "include/domain.h"

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "ru");
    try {
        domain::run(argv, argc);
    } catch (std::exception &e) {
        std::cerr << "Unforeseen error while running the program: " << e.what() << std::endl;
    } catch (...) { // this should never execute (meaning every thrown exception must be a std::exception derivative)
        std::cerr << "Unforeseen error while running the program." << std::endl;
    }
    return 0;
}