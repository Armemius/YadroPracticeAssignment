#include "parser/parser.hpp"

#include <charconv>
#include <cstdint>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

#include "simulation/machine.hpp"
#include "simulation/simulation.hpp"
#include "simulation/types.hpp"

namespace parser {

namespace {

constexpr int64_t MIN_PRODUCT_TYPE_COUNT = 1;
constexpr int64_t MAX_PRODUCT_TYPE_COUNT = 100;
constexpr int64_t MIN_MACHINE_COUNT = 1;
constexpr int64_t MAX_MACHINE_COUNT = 100;
constexpr int64_t MAX_OPERATION_TIME = 10'000;
constexpr int64_t MAX_PRODUCT_COUNT = 100'000;

std::vector<std::string> split_tokens(const std::string &line) {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int64_t parse_nonnegative_integer(const std::string &token, const std::string &line) {
    int64_t value = 0;
    const char *begin = token.data();
    const char *end = token.data() + token.size();
    const auto [position, error] = std::from_chars(begin, end, value);
    if (error != std::errc{} || position != end || value < 0) {
        throw InvalidInputLine(line);
    }
    return value;
}

std::vector<int64_t> parse_integer_line(const std::string &line) {
    std::vector<int64_t> values;
    for (const auto &token : split_tokens(line)) {
        values.push_back(parse_nonnegative_integer(token, line));
    }
    return values;
}

std::string read_line_or_throw(std::istream &input) {
    std::string line;
    if (!std::getline(input, line)) {
        throw InvalidInputLine("");
    }
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

void require_count(const std::vector<int64_t> &values, size_t expected, const std::string &line) {
    if (values.size() != expected) {
        throw InvalidInputLine(line);
    }
}

void require_range(int64_t value, int64_t min_value, int64_t max_value, const std::string &line) {
    if (value < min_value || value > max_value) {
        throw InvalidInputLine(line);
    }
}

bool has_non_whitespace(const std::string &line) {
    return line.find_first_not_of(" \t\r\n") != std::string::npos;
}

}  // namespace

InvalidInputLine::InvalidInputLine(std::string line) : ParserError("invalid input line"), line_(std::move(line)) {}

const std::string &InvalidInputLine::line() const noexcept {
    return line_;
}

sim::Simulation parse(std::istream &input, std::ostream &log) {
    const std::string header_line = read_line_or_throw(input);
    const std::vector<int64_t> header = parse_integer_line(header_line);
    require_count(header, 2, header_line);
    require_range(header[0], MIN_PRODUCT_TYPE_COUNT, MAX_PRODUCT_TYPE_COUNT, header_line);
    require_range(header[1], MIN_MACHINE_COUNT, MAX_MACHINE_COUNT, header_line);

    const auto product_type_count = static_cast<sim::product_type_t>(header[0]);
    const auto machine_count = static_cast<sim::machine_t>(header[1]);

    std::vector<std::vector<sim::optime_t>> operation_times(product_type_count - 1,
                                                            std::vector<sim::optime_t>(machine_count));
    for (sim::product_type_t operation = 0; operation + 1 < product_type_count; ++operation) {
        const std::string line = read_line_or_throw(input);
        const std::vector<int64_t> values = parse_integer_line(line);
        require_count(values, machine_count, line);
        for (sim::machine_t machine = 0; machine < machine_count; ++machine) {
            require_range(values[machine], 0, MAX_OPERATION_TIME, line);
            operation_times[operation][machine] = static_cast<sim::optime_t>(values[machine]);
        }
    }

    uint64_t total_products = 0;
    sim::product_index_t next_product_index = 0;
    std::vector<std::vector<sim::product_t>> initial_queues(machine_count);
    for (sim::machine_t machine = 0; machine < machine_count; ++machine) {
        const std::string line = read_line_or_throw(input);
        const std::vector<int64_t> values = parse_integer_line(line);
        if (values.empty()) {
            throw InvalidInputLine(line);
        }
        require_range(values[0], 0, MAX_PRODUCT_COUNT, line);

        const auto queue_size = static_cast<size_t>(values[0]);
        require_count(values, queue_size + 1, line);
        total_products += queue_size;
        if (total_products > MAX_PRODUCT_COUNT) {
            throw InvalidInputLine(line);
        }

        initial_queues[machine].reserve(queue_size);
        for (size_t item = 0; item < queue_size; ++item) {
            const int64_t product_type = values[item + 1];
            if (product_type_count == 1 || product_type > product_type_count - 2) {
                throw InvalidInputLine(line);
            }
            initial_queues[machine].push_back(
                {.index = next_product_index++, .type = static_cast<sim::product_type_t>(product_type)});
        }
    }

    std::string trailing_line;
    while (std::getline(input, trailing_line)) {
        if (!trailing_line.empty() && trailing_line.back() == '\r') {
            trailing_line.pop_back();
        }
        if (has_non_whitespace(trailing_line)) {
            throw InvalidInputLine(trailing_line);
        }
    }

    std::vector<sim::Machine> machines;
    machines.reserve(machine_count);
    for (sim::machine_t machine = 0; machine < machine_count; ++machine) {
        std::unordered_map<sim::product_type_t, sim::optime_t> optimes;
        for (sim::product_type_t operation = 0; operation + 1 < product_type_count; ++operation) {
            optimes.emplace(operation, operation_times[operation][machine]);
        }
        machines.emplace_back(machine, initial_queues[machine], std::move(optimes));
    }

    return sim::Simulation{std::move(machines), product_type_count, log};
}

}  // namespace parser
