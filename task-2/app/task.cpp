#include <cstdlib>
#include <fstream>
#include <iostream>

#include "parser/parser.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: task <input>\n";
        return EXIT_FAILURE;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Unable to open input file: " << argv[1] << "\n";
        return EXIT_FAILURE;
    }

    std::ofstream out{"result.txt"};
    if (!out) {
        std::cerr << "Unable to open output file: result.txt\n";
        return EXIT_FAILURE;
    }

    try {
        parser::parse(in, out).run();
    } catch (const parser::InvalidInputLine &error) {
        out << error.line() << "\n";
        return EXIT_FAILURE;
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
