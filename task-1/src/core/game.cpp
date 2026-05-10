#include "core/game.hpp"
#include <optional>
#include <stdexcept>
#include "core/player.hpp"
#include "core/resources.hpp"

namespace tvb::core {

Game::Game(std::unique_ptr<Player> player, std::unique_ptr<Dungeon> dungeon) {
    if (player == nullptr || dungeon == nullptr) {
        throw std::invalid_argument("Neither player and dungeon can be null");
    }
    player_ = std::move(player);
    dungeon_ = std::move(dungeon);
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

Game::RoomState Game::player_room_state() const {
    const auto &room = dungeon_->get_curent_room_info(*player_);
    return RoomState{.room_idx = room.idx(),
                     .resources = {.iron_amount = room.count(Resources::IRON),
                                   .gold_amount = room.count(Resources::GOLD),
                                   .gems_amount = room.count(Resources::GEM),
                                   .experience_amount = room.count(Resources::EXPERIENCE)},
                     .last_harvested_resource = last_harvested_resource_};
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
    last_harvested_resource_ = std::nullopt;
}

void Game::harvest(const Resource &resource) {
    dungeon_->harvest(*player_, resource);
    last_harvested_resource_ = resource.type();
}

}  // namespace tvb::core
