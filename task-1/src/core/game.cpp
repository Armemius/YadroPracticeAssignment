#include "core/game.hpp"

#include "core/dungeon.hpp"
#include "core/player.hpp"
#include "core/resources.hpp"

#include <cstdint>
#include <stdexcept>

namespace tvb::core {

Game::Game(std::unique_ptr<Player> player, std::unique_ptr<Dungeon> dungeon) {
    if (player == nullptr || dungeon == nullptr) {
        throw std::invalid_argument("Neither player and dungeon can be null");
    }
    player_ = std::move(player);
    dungeon_ = std::move(dungeon);
    dungeon_->init_player(*player_);
}

uint16_t Game::player_food() const noexcept {
    return player_->food();
}

bool Game::player_alive() const noexcept {
    return player_->alive();
}

uint16_t Game::player_room() const noexcept {
    return player_->room();
}

uint32_t Game::player_value() const noexcept {
    return player_->value();
}

uint16_t Game::player_amount(const Resource &resource) const {
    return player_->amount(resource);
}

uint16_t Game::resource_value(const Resource &resource) const noexcept {
    return player_->resource_value(resource);
}

Game::RoomState Game::player_room_state() const {
    const auto &room = dungeon_->get_curent_room_info(*player_);
    return RoomState{.room_idx = room.idx(),
                     .resources = {.iron_amount = room.count(Resources::IRON),
                                   .gold_amount = room.count(Resources::GOLD),
                                   .gems_amount = room.count(Resources::GEM),
                                   .experience_amount = room.count(Resources::EXPERIENCE)},
                     .harvested_resources = room.harvested_resources()};
}

Game::PlayerState Game::player_state() const noexcept {
    return PlayerState{.resources = {.iron_amount = player_->amount(Resources::IRON),
                                     .gold_amount = player_->amount(Resources::GOLD),
                                     .gems_amount = player_->amount(Resources::GEM),
                                     .experience_amount = player_->amount(Resources::EXPERIENCE)},
                       .total_value = player_->value()};
}

Dungeon::RoomView Game::get_room_info(uint8_t room) const {
    return dungeon_->get_available_room_info(*player_, room);
}

void Game::move_player(uint8_t room) {
    dungeon_->move(*player_, room);
}

void Game::harvest(const Resource &resource) {
    dungeon_->harvest(*player_, resource);
}

bool Game::free_harvest() const {
    return tvb::core::Dungeon::free_harvest(*player_);
}

bool Game::free_harvest(uint8_t room) const {
    return tvb::core::Dungeon::free_harvest(*player_, room);
}

const Dungeon::Room &Game::get_current_room() const {
    return dungeon_->get_curent_room_info(*player_);
}

RoomKnowledge Game::get_room_knowledge(uint8_t room) const {
    return tvb::core::Dungeon::get_room_knowledge(*player_, room);
}

std::pair<RoomIndexIterator, RoomIndexIterator> Game::known_rooms() const {
    return tvb::core::Dungeon::known_rooms(*player_);
}

std::pair<RoomIndexIterator, RoomIndexIterator> Game::visible_rooms() const {
    return tvb::core::Dungeon::visible_rooms(*player_);
}

std::pair<RoomIndexIterator, RoomIndexIterator> Game::visited_rooms() const {
    return tvb::core::Dungeon::visited_rooms(*player_);
}

std::pair<MergedRoomIndexIterator, MergedRoomIndexIterator> Game::nonvisited_rooms() const {
    return tvb::core::Dungeon::nonvisited_rooms(*player_);
}

}  // namespace tvb::core
