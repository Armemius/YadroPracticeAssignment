#pragma once

#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

#include "simulation/simulation.hpp"

namespace parser {

/// Base class for parser errors.
class ParserError : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/// Error for a malformed input line.
class InvalidInputLine final : public ParserError {
   public:
    /**
     * @brief Construct an error for the first invalid input line.
     *
     * @param line Original invalid input line.
     */
    explicit InvalidInputLine(std::string line);

    /**
     * @brief Returns the original invalid input line.
     *
     * @return const std::string& Invalid input line.
     */
    [[nodiscard]] const std::string &line() const noexcept;

   private:
    std::string line_;
};

/**
 * @brief Parses simulation input and builds a ready-to-run simulation.
 *
 * @param input Input stream with task data.
 * @param log Output stream for simulation events.
 * @return sim::Simulation Parsed simulation object.
 * @throws InvalidInputLine If a line has wrong token count, unreadable value, or out-of-range value.
 */
sim::Simulation parse(std::istream &input, std::ostream &log);

}  // namespace parser
