#include "core/dungeon.hpp"

#include "core/player.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <type_traits>

namespace tvb::core {

uint8_t Dungeon::Room::idx() const {
    return idx_;
}

bool Dungeon::Room::has(const Resource &resource) const {
    return resources_.contains(resource) && resources_.at(resource) > 0;
}

int Dungeon::Room::count(const Resource &resource) const {
    if (!resources_.contains(resource)) {
        return 0;
    }
    return resources_.at(resource);
}

const std::vector<uint8_t> &Dungeon::Room::adjacent_rooms() const noexcept {
    return adjacent_rooms_;
}

const std::unordered_map<Resource, uint8_t> &Dungeon::Room::resources() const noexcept {
    return resources_;
}

Dungeon::RoomView Dungeon::getRoom(const Player &player, uint8_t room) const {
    const Room &room_info = rooms_.at(room);
    auto level = static_cast<std::underlying_type_t<RoomKnowledge>>(player.knowledge_.access(room));
    constexpr auto UNKNOWN_LEVEL = static_cast<std::underlying_type_t<RoomKnowledge>>(RoomKnowledge::UNKNOWN);
    constexpr auto VISIBLE_LEVEL = static_cast<std::underlying_type_t<RoomKnowledge>>(RoomKnowledge::VISIBLE);
    constexpr auto VISITED_LEVEL = static_cast<std::underlying_type_t<RoomKnowledge>>(RoomKnowledge::VISITED);

    decltype(RoomView::idx) index = std::nullopt;
    decltype(RoomView::adjacent_rooms) adjacent_rooms = std::nullopt;
    decltype(RoomView::resources) resources = std::nullopt;

    if (level > UNKNOWN_LEVEL) {
        index = room_info.idx();
    }

    if (level >= VISIBLE_LEVEL) {
        adjacent_rooms = std::cref(room_info.adjacent_rooms());
    }

    if (level >= VISITED_LEVEL) {
        resources = std::cref(room_info.resources());
    }

    return {.idx = index, .adjacent_rooms = adjacent_rooms, .resources = resources};
}

void Dungeon::move(Player &player, uint8_t target_room) {
    if (!player.alive()) [[unlikely]] {
        throw std::logic_error("Player is not alive");
    }
    if (player.food() == 0) [[unlikely]] {
        throw std::logic_error("Player cannot move");
    }
    if (!rooms_.contains(player.room())) [[unlikely]] {
        throw std::logic_error("Player's room is non-existent");
    }

    const auto &adjacent_rooms = rooms_.at(player.room()).adjacent_rooms();
    bool move_possible = std::ranges::binary_search(adjacent_rooms, target_room);
    if (!move_possible) [[unlikely]] {
        throw std::logic_error("Impossible move");
    }
    --player.food_left_;
    player.current_room_idx_ = target_room;

    // TODO: armemius - implement room knowledge update
}

void Dungeon::harvest(Player &player, const Resource &resource) {
    if (!player.alive()) [[unlikely]] {
        throw std::logic_error("Player is not alive");
    }
    if (player.food() == 0) [[unlikely]] {
        throw std::logic_error("Player cannot harvest resources");
    }
    if (!rooms_.contains(player.room())) [[unlikely]] {
        throw std::logic_error("Player's room is non-existent");
    }
    auto &room = rooms_.at(player.room());
    if (!room.has(resource)) {
        throw std::logic_error("No available resources");
    }
    if (player.knowledge_.harvested_rooms_.contains(player.room())) [[likely]] {
        --player.food_left_;
    } else [[unlikely]] {
        player.knowledge_.harvested_rooms_.insert(player.room());
    }
    --room.resources_.at(resource);
    ++player.harvested_resources_[resource];
}

}  // namespace tvb::core
