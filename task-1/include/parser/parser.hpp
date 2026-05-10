#pragma once

#include <core/game.hpp>
#include <istream>
#include <stdexcept>
#include <string>

namespace tvb::parser {

class ParserError : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

class InvalidInputLine final : public ParserError {
   public:
    explicit InvalidInputLine(std::string line);

    [[nodiscard]] const std::string &line() const noexcept;

   private:
    std::string line_;
};

/**
 * @brief Parses input stream to the game object
 * 
 * @param in input stream to parse
 * @return core::Game Game object if parsing was successful
 * @exception InvalidInputLine throws if input format is invalid
 */
core::Game parse(std::istream &in);

}  // namespace tvb::parser
