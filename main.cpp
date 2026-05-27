/**
 * main.cpp — Native CLI for Math Relation Analyser
 *
 * Build:  g++ -std=c++17 -O2 main.cpp -o math-analyser
 * Usage:  math-analyser "1,2,3" "(1,1); (1,2); (2,2)"
 */

#include "relation_engine.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: math-analyser <set_A> <relation_R>\n"
                  << "Example: math-analyser \"1,2,3\" \"(1,1); (1,2); (2,3)\"\n";
        return 1;
    }
    try {
        std::cout << math::analyse_json(argv[1], argv[2]) << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "{\"error\":\"" << e.what() << "\"}\n";
        return 2;
    }
}
