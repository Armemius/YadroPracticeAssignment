#include "core/dungeon.hpp"

#include "core/player.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <type_traits>

namespace tvb::core {

Dungeon::Room::Room(uint8_t idx, std::unordered_map<Resource, uint8_t> resources, std::vector<uint8_t> adjacent_rooms)
    : idx_(idx), resources_(std::move(resources)), adjacent_rooms_(std::move(adjacent_rooms)) {
    std::ranges::sort(adjacent_rooms_);
}

uint8_t Dungeon::Room::idx() const {
    return idx_;
}

bool Dungeon::Room::has(const Resource &resource) const {
    return resources_.contains(resource) && resources_.at(resource) > 0;
}

uint16_t Dungeon::Room::count(const Resource &resource) const {
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

const std::unordered_set<Resource> &Dungeon::Room::harvested_resources() const noexcept {
    return harvested_resources_;
}

Dungeon::RoomView Dungeon::get_available_room_info(const Player &player, uint8_t room) const {
    const Room &room_info = rooms_.at(room);
    auto level = static_cast<std::underlying_type_t<RoomKnowledge>>(player.knowledge_.access(room));
    constexpr auto KNOWN_LEVEL = static_cast<std::underlying_type_t<RoomKnowledge>>(RoomKnowledge::KNOWN);
    constexpr auto VISIBLE_LEVEL = static_cast<std::underlying_type_t<RoomKnowledge>>(RoomKnowledge::VISIBLE);
    constexpr auto VISITED_LEVEL = static_cast<std::underlying_type_t<RoomKnowledge>>(RoomKnowledge::VISITED);

    if (room == player.room()) {
        level = std::max(level, VISITED_LEVEL);
    }

    decltype(RoomView::idx) index = std::nullopt;
    decltype(RoomView::adjacent_rooms) adjacent_rooms = std::nullopt;
    decltype(RoomView::resources) resources = std::nullopt;

    if (level >= KNOWN_LEVEL) {
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

const Dungeon::Room &Dungeon::get_curent_room_info(const Player &player) const {
    if (!rooms_.contains(player.room())) [[unlikely]] {
        throw std::logic_error("Player's room is non-existent");
    }
    return rooms_.at(player.room());
}

RoomKnowledge Dungeon::get_room_knowledge(const Player &player, uint8_t room) {
    return player.knowledge_.access(room);
}

Dungeon::Dungeon(std::unordered_map<uint8_t, Room> rooms) : rooms_(std::move(rooms)) {
    // Connect adjacent rooms
    auto insert_room_if_not_present = [](std::vector<uint8_t> &rooms, uint8_t idx) {
        auto pos = std::ranges::lower_bound(rooms, idx);
        if (pos == rooms.end() || *pos != idx) {
            rooms.insert(pos, idx);
        }
    };

    for (auto &[src_idx, room] : rooms_) {
        for (auto dest_idx : room.adjacent_rooms()) {
            insert_room_if_not_present(rooms_.at(dest_idx).adjacent_rooms_, src_idx);
        }
    }
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
    if (!rooms_.contains(target_room)) [[unlikely]] {
        throw std::logic_error("Target room is non-existent");
    }

    const auto &adjacent_rooms = rooms_.at(player.room()).adjacent_rooms();
    bool move_possible = std::ranges::find(adjacent_rooms, target_room) != adjacent_rooms.end();
    if (!move_possible) [[unlikely]] {
        throw std::logic_error("Impossible move");
    }
    update_knowledge(player, player.room());

    --player.food_left_;
    player.current_room_idx_ = target_room;
    update_knowledge(player, target_room);
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
    update_knowledge(player, player.room());
    if (player.knowledge_.access(player.room()) != RoomKnowledge::VISITED) [[unlikely]] {
        throw std::logic_error("Player cannot harvest resources in a non-visited room");
    }

    auto &room = rooms_.at(player.room());
    if (!room.has(resource)) {
        throw std::logic_error("No available resources");
    }
    auto [_, first_harvest] = player.knowledge_.harvested_rooms_.insert(player.room());
    if (!first_harvest) {
        --player.food_left_;
    }

    player.harvested_resources_[resource] += room.resources_.at(resource);
    room.resources_.at(resource) = 0;
    room.harvested_resources_.insert(resource);
}

void Dungeon::init_player(Player &player) {
    player.current_room_idx_ = 0;
    update_knowledge(player, 0);
}

std::pair<RoomIndexIterator, RoomIndexIterator> Dungeon::known_rooms(const Player &player) {
    return player.knowledge_.known_rooms();
}

std::pair<RoomIndexIterator, RoomIndexIterator> Dungeon::visible_rooms(const Player &player) {
    return player.knowledge_.visible_rooms();
}

std::pair<RoomIndexIterator, RoomIndexIterator> Dungeon::visited_rooms(const Player &player) {
    return player.knowledge_.visited_rooms();
}

std::pair<MergedRoomIndexIterator, MergedRoomIndexIterator> Dungeon::nonvisited_rooms(const Player &player) {
    return player.knowledge_.nonvisited_rooms();
}

void Dungeon::update_knowledge(Player &player, uint8_t room) const {
    const auto room_iter = rooms_.find(room);
    if (room_iter == rooms_.end()) [[unlikely]] {
        throw std::logic_error("Room is non-existent");
    }

    player.knowledge_.promote(room, RoomKnowledge::VISITED);

    for (uint8_t visible_room : room_iter->second.adjacent_rooms()) {
        player.knowledge_.promote(visible_room, RoomKnowledge::VISIBLE);

        const auto visible_room_iter = rooms_.find(visible_room);
        if (visible_room_iter == rooms_.end()) {
            continue;
        }

        for (uint8_t known_room : visible_room_iter->second.adjacent_rooms()) {
            player.knowledge_.promote(known_room, RoomKnowledge::KNOWN);
        }
    }
}

}  // namespace tvb::core
