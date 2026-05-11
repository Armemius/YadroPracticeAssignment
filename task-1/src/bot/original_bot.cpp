#include "bot/original_bot.hpp"

#include "core/player.hpp"
#include "core/resources.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace tvb::bot {

OriginalBot::OriginalBot(std::shared_ptr<core::Game> game, std::ostream &out)
    : BotBase(std::move(game), out), food_threshold_((OriginalBot::game().player_food() + 1) / 2) {}

void OriginalBot::next() {
    if (!returning_ && game().player_food() > food_threshold_) {
        next_explore();
    } else {
        returning_ = true;
        next_backtrack();
    }
}

bool OriginalBot::finished() const {
    return returning_ && game().player_room() == 0;
}

void OriginalBot::next_explore() {
    if (try_harvest_best_free_resource()) {
        return;
    }

    if (current_path_.empty()) {
        set_current_path(path_to_next_unvisited());
    }

    if (current_path_.empty()) {
        returning_ = true;
        next_backtrack();
        return;
    }

    move(current_path_.front());
    current_path_.pop();
}

void OriginalBot::next_backtrack() {
    if (game().player_room() == 0) {
        return;
    }

    auto return_path = path_to_entrance();
    if (return_path.empty()) {
        throw std::logic_error("No path to entrance");
    }

    if (try_harvest_best_extra_resource()) {
        return;
    }

    move(return_path.front());
}

bool OriginalBot::try_harvest_best_free_resource() {
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

bool OriginalBot::try_harvest_best_extra_resource() {
    auto return_path = path_to_entrance();
    if (!game().free_harvest() && game().player_food() <= return_path.size()) {
        return false;
    }

    const core::Resource *resource = best_available_resource();
    if (resource == nullptr) {
        return false;
    }

    harvest(*resource);
    return true;
}

const core::Resource *OriginalBot::best_available_resource() const {
    const auto room_state = game().player_room_state();
    const core::Resource *best_resource = nullptr;
    uint16_t best_value = 0;

    auto try_resource = [&](const core::Resource &resource, uint16_t amount) {
        if (amount == 0) {
            return;
        }

        const uint16_t value = game().resource_value(resource);
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

std::vector<uint8_t> OriginalBot::path_to_next_unvisited() const {
    std::vector<uint8_t> best_path;
    auto [begin, end] = game().nonvisited_rooms();
    for (auto it = begin; it != end; ++it) {
        auto path = path_to_unvisited(*it);
        if (path.empty()) {
            continue;
        }
        if (best_path.empty() || path.size() < best_path.size()) {
            best_path = std::move(path);
        }
    }
    return best_path;
}

std::vector<uint8_t> OriginalBot::path_to_entrance() const {
    return shortest_path(0, false);
}

std::vector<uint8_t> OriginalBot::path_to_unvisited(uint8_t target_room) const {
    return shortest_path(target_room, true);
}

std::vector<uint8_t> OriginalBot::shortest_path(uint8_t target_room, bool target_may_be_unvisited) const {
    if (game().player_room() == target_room) {
        return {};
    }

    std::queue<uint8_t> rooms;
    std::unordered_map<uint8_t, uint8_t> previous_room;
    rooms.push(static_cast<uint8_t>(game().player_room()));
    previous_room.emplace(static_cast<uint8_t>(game().player_room()), std::numeric_limits<uint8_t>::max());

    while (!rooms.empty()) {
        const uint8_t current_room = rooms.front();
        rooms.pop();

        auto room_info = game().get_room_info(current_room);
        if (!room_info.adjacent_rooms.has_value()) {
            continue;
        }

        for (uint8_t adjacent_room : room_info.adjacent_rooms->get()) {
            const auto knowledge = game().get_room_knowledge(adjacent_room);
            if (adjacent_room == target_room) {
                if (target_may_be_unvisited || knowledge == core::RoomKnowledge::VISITED) {
                    previous_room.emplace(adjacent_room, current_room);
                    std::vector<uint8_t> path;
                    for (uint8_t room = adjacent_room; room != game().player_room(); room = previous_room.at(room)) {
                        path.push_back(room);
                    }
                    std::ranges::reverse(path);
                    return path;
                }
            }

            if (knowledge != core::RoomKnowledge::VISITED || previous_room.contains(adjacent_room)) {
                continue;
            }
            previous_room.emplace(adjacent_room, current_room);
            rooms.push(adjacent_room);
        }
    }

    return {};
}

void OriginalBot::set_current_path(const std::vector<uint8_t> &path) {
    current_path_ = {};
    for (uint8_t room : path) {
        current_path_.push(room);
    }
}

}  // namespace tvb::bot
