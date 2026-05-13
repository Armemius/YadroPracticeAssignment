#include "parser/parser.hpp"

#include "core/dungeon.hpp"
#include "core/player.hpp"
#include "core/resources.hpp"

#include <charconv>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tvb::parser {

InvalidInputLine::InvalidInputLine(std::string line) : ParserError(line), line_(std::move(line)) {}

const std::string &InvalidInputLine::line() const noexcept {
    return line_;
}

namespace {

[[noreturn]] void fail(const std::string &line) {
    throw InvalidInputLine(line);
}

int parse_int_token(std::string_view token, int min_value, int max_value, const std::string &line) {
    if (token.empty()) {
        fail(line);
    }

    int value = 0;
    const auto *begin = token.data();
    const auto *end = begin + token.size();
    auto [ptr, ec] = std::from_chars(begin, end, value);
    if (ec != std::errc{} || ptr != end || value < min_value || value > max_value) {
        fail(line);
    }

    return value;
}

uint8_t parse_uint8_token(std::string_view token, int min_value, int max_value, const std::string &line) {
    return static_cast<uint8_t>(parse_int_token(token, min_value, max_value, line));
}

std::string next_token(std::istringstream &line_in, const std::string &line) {
    std::string token;
    if (!(line_in >> token)) {
        fail(line);
    }
    return token;
}

bool has_extra_token(std::istringstream &line_in) {
    std::string token;
    return static_cast<bool>(line_in >> token);
}

std::vector<uint8_t> parse_adjacent_rooms(std::string_view token, uint8_t max_room, const std::string &line) {
    if (token.empty()) {
        fail(line);
    }

    std::vector<uint8_t> adjacent_rooms;
    std::unordered_set<uint8_t> seen_rooms;
    size_t start = 0;
    while (start <= token.size()) {
        const size_t end = token.find(',', start);
        const auto part = token.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);
        const uint8_t room = parse_uint8_token(part, 0, max_room, line);
        if (!seen_rooms.insert(room).second) {
            fail(line);
        }
        adjacent_rooms.push_back(room);

        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
        if (start == token.size()) {
            fail(line);
        }
    }

    return adjacent_rooms;
}

core::ResourceType parse_resource_type(std::string_view token, const std::string &line) {
    if (token == "iron") {
        return core::ResourceType::IRON;
    }
    if (token == "gold") {
        return core::ResourceType::GOLD;
    }
    if (token == "gems") {
        return core::ResourceType::GEM;
    }
    if (token == "exp") {
        return core::ResourceType::EXPERIENCE;
    }
    fail(line);
}

uint8_t parse_rooms_count(std::istream &in, std::string &line) {
    if (!std::getline(in, line)) {
        fail(line);
    }

    std::istringstream line_in{line};
    const uint8_t rooms_count = parse_uint8_token(next_token(line_in, line), 1, 255, line);
    if (has_extra_token(line_in)) {
        fail(line);
    }

    return rooms_count;
}

core::Dungeon::Room parse_room(std::istream &in, std::string &line, uint8_t max_room) {
    if (!std::getline(in, line)) {
        fail(line);
    }

    std::istringstream line_in{line};
    const uint8_t room_index = parse_uint8_token(next_token(line_in, line), 0, max_room, line);
    auto adjacent_rooms = parse_adjacent_rooms(next_token(line_in, line), max_room, line);

    uint8_t iron = 0;
    uint8_t gold = 0;
    uint8_t gems = 0;
    uint8_t experience = 0;

    std::string token;
    if (line_in >> token) {
        iron = parse_uint8_token(token, 0, 255, line);
        gold = parse_uint8_token(next_token(line_in, line), 0, 255, line);
        gems = parse_uint8_token(next_token(line_in, line), 0, 255, line);
        experience = parse_uint8_token(next_token(line_in, line), 0, 255, line);
        if (has_extra_token(line_in)) {
            fail(line);
        }
    } else if (room_index != 0) {
        fail(line);
    }

    std::unordered_map<core::Resource, uint8_t> resources{
        {core::Resources::IRON, iron},
        {core::Resources::GOLD, gold},
        {core::Resources::GEM, gems},
        {core::Resources::EXPERIENCE, experience},
    };

    return core::Dungeon::Room{room_index, std::move(resources), std::move(adjacent_rooms)};
}

std::pair<uint8_t, core::ResourceType> parse_player(std::istream &in, std::string &line) {
    if (!std::getline(in, line)) {
        fail(line);
    }

    std::istringstream line_in{line};
    const uint8_t food = parse_uint8_token(next_token(line_in, line), 2, 255, line);
    const core::ResourceType target_resource = parse_resource_type(next_token(line_in, line), line);
    if (has_extra_token(line_in)) {
        fail(line);
    }

    return {food, target_resource};
}

}  // namespace

core::Game parse(std::istream &in) {
    std::string line;
    line.reserve(256);

    const uint8_t rooms_count = parse_rooms_count(in, line);
    std::unordered_map<uint8_t, core::Dungeon::Room> rooms;
    rooms.reserve(static_cast<size_t>(rooms_count) + 1);

    for (int i = 0; i <= rooms_count; ++i) {
        auto room = parse_room(in, line, rooms_count);
        const uint8_t room_index = room.idx();
        if (!rooms.emplace(room_index, std::move(room)).second) {
            fail(line);
        }
    }

    const auto [food, target_resource] = parse_player(in, line);

    if (std::getline(in, line)) {
        fail(line);
    }

    auto player = std::make_unique<core::Player>(target_resource, food);
    auto dungeon = std::make_unique<core::Dungeon>(std::move(rooms));
    return core::Game{std::move(player), std::move(dungeon)};
}

}  // namespace tvb::parser
