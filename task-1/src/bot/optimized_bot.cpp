#include "bot/optimized_bot.hpp"

#include "core/player.hpp"
#include "core/resources.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tvb::bot {
namespace {

constexpr uint8_t ENTRANCE_ROOM = 0;

// Those are euristics for the bot determined by experiments :)
constexpr double EXPECTED_WEIGHT = 1.0;
constexpr double EXPLORATION_WEIGHT = 1.35;
constexpr double MOVEMENT_COST_WEIGHT = 8.0;
constexpr double RETURN_RISK_WEIGHT = 2.5;
constexpr double CORRIDOR_BONUS = 500.0;
constexpr uint8_t BONUS_DEPTH_LIMIT = 3;
constexpr uint16_t RETURN_ROUTE_EXTRA_DEPTH = 4;

}  // namespace

OptimizedBot::OptimizedBot(std::shared_ptr<core::Game> game, std::ostream &out) : BotBase(std::move(game), out) {}

void OptimizedBot::next() {
    remember_current_room_value();

    if (try_harvest_best_free_resource()) {
        return;
    }

    if (should_return()) {
        returning_ = true;
        next_return();
        return;
    }

    next_explore();
}

bool OptimizedBot::finished() const {
    return finished_ || (returning_ && game().player_room() == ENTRANCE_ROOM);
}

void OptimizedBot::next_explore() {
    if (auto step = best_adjacent_exploration_step(); step.has_value()) {
        move(*step);
        return;
    }

    if (auto path = best_exploration_path(); path.has_value() && !path->empty()) {
        move(path->front());
        return;
    }

    if (try_harvest_best_extra_resource()) {
        return;
    }

    returning_ = true;
    next_return();
}

void OptimizedBot::next_return() {
    if (game().player_room() == ENTRANCE_ROOM) {
        finish_at_entrance();
        return;
    }

    if (try_harvest_best_free_resource()) {
        return;
    }

    if (try_harvest_best_extra_resource()) {
        return;
    }

    auto route = best_return_route();
    if (!route.has_value() || route->rooms.empty()) {
        finish_at_entrance();
        return;
    }

    move(route->rooms.front());
}

void OptimizedBot::remember_current_room_value() {
    const auto room = static_cast<uint8_t>(game().player_room());
    if (first_visit_values_.contains(room)) {
        return;
    }
    first_visit_values_.emplace(room, current_room_value());
}

void OptimizedBot::finish_at_entrance() {
    returning_ = true;
    finished_ = true;
    log_results();
}

bool OptimizedBot::should_return() const {
    if (game().player_room() == ENTRANCE_ROOM) {
        return best_adjacent_exploration_step() == std::nullopt && best_exploration_path() == std::nullopt;
    }

    const uint16_t distance = return_distance();
    if (distance == std::numeric_limits<uint16_t>::max()) {
        return false;
    }
    return game().player_food() <= distance;
}

bool OptimizedBot::try_harvest_best_free_resource() {
    if (!game().free_harvest()) {
        return false;
    }

    const core::Resource *resource = best_available_resource();
    if (resource == nullptr) {
        return false;
    }

    harvest(*resource);
    return true;
}

bool OptimizedBot::try_harvest_best_extra_resource() {
    if (game().free_harvest()) {
        return false;
    }

    const uint16_t distance = return_distance();
    if (distance == std::numeric_limits<uint16_t>::max() || game().player_food() <= distance) {
        return false;
    }

    const core::Resource *resource = best_available_resource();
    if (resource == nullptr) {
        return false;
    }

    harvest(*resource);
    return true;
}

const core::Resource *OptimizedBot::best_available_resource() const {
    const auto room_state = game().player_room_state();
    const core::Resource *best_resource = nullptr;
    uint16_t best_value = 0;

    auto try_resource = [&](const core::Resource &resource, uint16_t amount) {
        if (amount == 0) {
            return;
        }

        const uint16_t value = game().resource_value(resource) * amount;
        if (best_resource == nullptr || value > best_value) {
            best_resource = &resource;
            best_value = value;
        }
    };

    try_resource(core::Resources::GEM, room_state.resources.gems_amount);
    try_resource(core::Resources::GOLD, room_state.resources.gold_amount);
    try_resource(core::Resources::IRON, room_state.resources.iron_amount);
    try_resource(core::Resources::EXPERIENCE, room_state.resources.experience_amount);

    return best_resource;
}

std::optional<uint8_t> OptimizedBot::best_adjacent_exploration_step() const {
    const auto current_room = static_cast<uint8_t>(game().player_room());
    auto adjacent = adjacent_rooms(current_room);
    if (!adjacent.has_value()) {
        return std::nullopt;
    }

    std::optional<uint8_t> best_room;
    double best_score = -std::numeric_limits<double>::infinity();

    for (uint8_t room : *adjacent) {
        if (game().room_knowledge(room) == core::RoomKnowledge::VISITED || !can_use_room(room) ||
            !can_safely_move_to(room)) {
            continue;
        }

        const double score = exploration_score(room, 1);
        if (!best_room.has_value() || score > best_score ||
            (std::abs(score - best_score) < 0.001 && room < *best_room)) {
            best_room = room;
            best_score = score;
        }
    }

    return best_room;
}

std::optional<std::vector<uint8_t>> OptimizedBot::best_exploration_path() const {
    std::optional<std::vector<uint8_t>> best_path;
    double best_score = -std::numeric_limits<double>::infinity();
    uint8_t best_target = std::numeric_limits<uint8_t>::max();

    auto [begin, end] = game().nonvisited_rooms();
    for (auto it = begin; it != end; ++it) {
        const uint8_t target = *it;
        if (!can_use_room(target)) {
            continue;
        }

        auto path = shortest_path(target);
        if (!path.has_value() || path->empty()) {
            continue;
        }

        const auto movement_cost = static_cast<uint16_t>(path->size());
        if (movement_cost >= game().player_food()) {
            continue;
        }

        const double score = exploration_score(target, movement_cost);
        if (!best_path.has_value() || score > best_score ||
            (std::abs(score - best_score) < 0.001 &&
             (target < best_target || (target == best_target && path->front() < best_path->front())))) {
            best_path = std::move(path);
            best_score = score;
            best_target = target;
        }
    }

    return best_path;
}

std::optional<OptimizedBot::Route> OptimizedBot::best_return_route() const {
    const uint16_t food = game().player_food();
    if (game().player_room() == ENTRANCE_ROOM) {
        return Route{};
    }

    auto shortest = shortest_path(ENTRANCE_ROOM);
    if (!shortest.has_value()) {
        return std::nullopt;
    }

    const auto route_limit = static_cast<uint16_t>(shortest->size() + RETURN_ROUTE_EXTRA_DEPTH);
    const auto max_depth = static_cast<uint16_t>(std::min<uint16_t>(food, route_limit));
    std::optional<Route> best_route;
    std::vector<uint8_t> path;
    std::unordered_set<uint8_t> used;
    used.insert(static_cast<uint8_t>(game().player_room()));

    auto better_route = [](const Route &lhs, const Route &rhs) {
        if (lhs.nonvisited_rooms != rhs.nonvisited_rooms) {
            return lhs.nonvisited_rooms > rhs.nonvisited_rooms;
        }
        if (lhs.expected_value != rhs.expected_value) {
            return lhs.expected_value > rhs.expected_value;
        }
        if (lhs.rooms.size() != rhs.rooms.size()) {
            return lhs.rooms.size() < rhs.rooms.size();
        }
        return !lhs.rooms.empty() && !rhs.rooms.empty() && lhs.rooms.front() < rhs.rooms.front();
    };

    std::function<void(uint8_t)> dfs = [&](uint8_t room) {
        if (path.size() > max_depth) {
            return;
        }
        if (room == ENTRANCE_ROOM) {
            Route route{.rooms = path};
            for (uint8_t path_room : route.rooms) {
                if (game().room_knowledge(path_room) != core::RoomKnowledge::VISITED) {
                    ++route.nonvisited_rooms;
                    route.expected_value += expected_room_value(path_room);
                } else {
                    route.expected_value += known_room_value(path_room);
                }
            }
            if (!best_route.has_value() || better_route(route, *best_route)) {
                best_route = std::move(route);
            }
            return;
        }

        auto adjacent = adjacent_rooms(room);
        if (!adjacent.has_value()) {
            return;
        }

        for (uint8_t next_room : *adjacent) {
            if (used.contains(next_room) || !can_use_room(next_room)) {
                continue;
            }
            used.insert(next_room);
            path.push_back(next_room);
            dfs(next_room);
            path.pop_back();
            used.erase(next_room);
        }
    };

    dfs(static_cast<uint8_t>(game().player_room()));
    return best_route;
}

std::optional<std::vector<uint8_t>> OptimizedBot::shortest_path(uint8_t target_room) const {
    return shortest_path_from(static_cast<uint8_t>(game().player_room()), target_room);
}

std::optional<std::vector<uint8_t>> OptimizedBot::shortest_path_from(uint8_t source_room, uint8_t target_room) const {
    if (source_room == target_room) {
        return std::vector<uint8_t>{};
    }

    std::queue<uint8_t> rooms;
    std::unordered_map<uint8_t, uint8_t> previous_room;
    rooms.push(source_room);
    previous_room.emplace(source_room, std::numeric_limits<uint8_t>::max());

    while (!rooms.empty()) {
        const uint8_t current_room = rooms.front();
        rooms.pop();

        auto adjacent = adjacent_rooms(current_room);
        if (!adjacent.has_value()) {
            continue;
        }

        for (uint8_t next_room : *adjacent) {
            if (previous_room.contains(next_room) || !can_use_room(next_room)) {
                continue;
            }

            previous_room.emplace(next_room, current_room);
            if (next_room == target_room) {
                std::vector<uint8_t> path;
                for (uint8_t room = next_room; room != source_room; room = previous_room.at(room)) {
                    path.push_back(room);
                }
                std::ranges::reverse(path);
                return path;
            }

            if (adjacent_rooms(next_room).has_value()) {
                rooms.push(next_room);
            }
        }
    }

    return std::nullopt;
}

std::optional<std::vector<uint8_t>> OptimizedBot::adjacent_rooms(uint8_t room) const {
    auto room_info = game().room_info(room);
    if (!room_info.adjacent_rooms.has_value()) {
        return std::nullopt;
    }
    return room_info.adjacent_rooms->get();
}

bool OptimizedBot::can_use_room(uint8_t room) const {
    return game().room_knowledge(room) != core::RoomKnowledge::UNKNOWN ||
           room == static_cast<uint8_t>(game().player_room());
}

bool OptimizedBot::can_safely_move_to(uint8_t room) const {
    if (game().player_food() == 0) {
        return false;
    }

    const auto food_after_move = static_cast<uint16_t>(game().player_food() - 1);
    // NOLINTNEXTLINE(readability-suspicious-call-argument)
    auto return_path = shortest_path_from(room, ENTRANCE_ROOM);
    if (!return_path.has_value()) {
        const uint16_t current_distance = return_distance();
        return current_distance != std::numeric_limits<uint16_t>::max() && game().player_food() > current_distance;
    }
    return food_after_move >= return_path->size();
}

double OptimizedBot::exploration_score(uint8_t room, uint16_t movement_cost) const {
    const double expected_value = static_cast<double>(expected_room_value(room)) * EXPECTED_WEIGHT;
    const double bonus = exploration_bonus(room, 0) * EXPLORATION_WEIGHT;
    // NOLINTNEXTLINE(readability-suspicious-call-argument)
    const auto return_path = shortest_path_from(room, ENTRANCE_ROOM);
    const double return_risk =
        return_path.has_value() ? static_cast<double>(return_path->size()) : static_cast<double>(return_distance() + 2);

    return expected_value + bonus - (static_cast<double>(movement_cost) * MOVEMENT_COST_WEIGHT) -
           (return_risk * RETURN_RISK_WEIGHT);
}

// NOLINTNEXTLINE(misc-no-recursion)
double OptimizedBot::exploration_bonus(uint8_t room, uint8_t depth) const {
    if (depth >= BONUS_DEPTH_LIMIT) {
        return 0.0;
    }

    auto adjacent = adjacent_rooms(room);
    if (!adjacent.has_value()) {
        return static_cast<double>(average_first_visit_value()) * 0.25;
    }

    uint16_t nonvisited_count = 0;
    uint16_t visited_count = 0;
    double neighbor_bonus = 0.0;
    for (uint8_t next_room : *adjacent) {
        if (game().room_knowledge(next_room) == core::RoomKnowledge::VISITED) {
            ++visited_count;
        } else if (can_use_room(next_room)) {
            ++nonvisited_count;
            neighbor_bonus += static_cast<double>(expected_room_value(next_room)) / static_cast<double>(depth + 2);
            neighbor_bonus += exploration_bonus(next_room, static_cast<uint8_t>(depth + 1)) * 0.5;
        }
    }

    const double degree_bonus = static_cast<double>(adjacent->size()) * 2.0;
    const double frontier_bonus = static_cast<double>(nonvisited_count) * 6.0;
    const double corridor_bonus = (visited_count == 1 && nonvisited_count == 1) ? CORRIDOR_BONUS : 0.0;
    return degree_bonus + frontier_bonus + corridor_bonus + neighbor_bonus;
}

uint32_t OptimizedBot::expected_room_value(uint8_t room) const {
    uint32_t value = known_room_value(room);
    if (value == 0 && game().room_knowledge(room) != core::RoomKnowledge::VISITED) {
        value = average_first_visit_value();
    }

    if (game().room_knowledge(room) != core::RoomKnowledge::VISITED) {
        value *= 2;
    }
    return value;
}

uint32_t OptimizedBot::known_room_value(uint8_t room) const {
    auto room_info = game().room_info(room);
    if (!room_info.resources.has_value()) {
        return 0;
    }

    uint32_t value = 0;
    for (const auto &[resource, amount] : room_info.resources->get()) {
        value += static_cast<uint32_t>(game().resource_value(resource)) * amount;
    }
    return value;
}

uint32_t OptimizedBot::current_room_value() const {
    const auto room_state = game().player_room_state();
    return (static_cast<uint32_t>(game().resource_value(core::Resources::IRON)) * room_state.resources.iron_amount) +
           (static_cast<uint32_t>(game().resource_value(core::Resources::GOLD)) * room_state.resources.gold_amount) +
           (static_cast<uint32_t>(game().resource_value(core::Resources::GEM)) * room_state.resources.gems_amount) +
           (static_cast<uint32_t>(game().resource_value(core::Resources::EXPERIENCE)) *
            room_state.resources.experience_amount);
}

uint32_t OptimizedBot::average_first_visit_value() const {
    if (first_visit_values_.empty()) {
        return std::max<uint32_t>(game().resource_value(core::Resources::GEM),
                                  game().resource_value(core::Resources::GOLD));
    }

    uint32_t total_value = 0;
    for (const auto &[_, value] : first_visit_values_) {
        total_value += value;
    }
    return total_value / static_cast<uint32_t>(first_visit_values_.size());
}

uint16_t OptimizedBot::return_distance() const {
    auto path = shortest_path(ENTRANCE_ROOM);
    if (!path.has_value()) {
        return std::numeric_limits<uint16_t>::max();
    }
    return static_cast<uint16_t>(path->size());
}

}  // namespace tvb::bot
