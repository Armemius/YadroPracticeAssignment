#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>

#include "bot/bot_base.hpp"
#include "bot/optimized_bot.hpp"
#include "core/game.hpp"
#include "parser/parser.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: optimized_task <input>\n";
        return EXIT_FAILURE;
    }

    std::ifstream in{argv[1]};
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
        auto game = std::make_shared<tvb::core::Game>(tvb::parser::parse(in));
        std::unique_ptr<tvb::bot::BotBase> bot = std::make_unique<tvb::bot::OptimizedBot>(game, out);
        bot->run();
    } catch (const tvb::parser::ParserError &ex) {
        out << ex.what() << "\n";
        return EXIT_FAILURE;
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
