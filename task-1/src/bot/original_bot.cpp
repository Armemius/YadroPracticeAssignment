#include "bot/original_bot.hpp"

namespace tvb::bot {

OriginalBot::OriginalBot(std::shared_ptr<core::Game> game, std::ostream &out)
    : BotBase(std::move(game), out), food_threshold_(OriginalBot::game().player_food() / 2) {}

void OriginalBot::next() {
    auto current_room = game().get_room_info(game().player_room());
    move(current_room.adjacent_rooms.value().get().at(0));
}

bool OriginalBot::finished() const {
    return game().player_room() == 0 && game().player_food() < food_threshold_;
}

}  // namespace tvb::bot