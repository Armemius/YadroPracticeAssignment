#include "bot/bot_base.hpp"
#include <cstdint>
#include <utility>
#include "core/resources.hpp"

namespace tvb::bot {

BotBase::BotBase(std::shared_ptr<core::Game> game, std::ostream &out) : game_(std::move(game)), out_(out) {}

void BotBase::run() {
    while (!finished() && game_->player_alive()) {
        next();
    }
}

void BotBase::move(uint8_t room) {
    game_->move_player(room);
    log_move_action(room);
    if (room != 0) {
        log_room_state();
    } else {
        log_results();
    }
}

void BotBase::harvest(const core::Resource &resource) {
    game_->harvest(resource);
    log_harvest_action(resource.type());
    log_room_state();
}

void BotBase::log_room_state() {
    auto state = game_->player_room_state();
    out_ << "state " << state.room_idx << " ";

    auto print_resource = [&](const core::Resource &resource, uint16_t value, const char *sep = " ") {
        if (state.harvested_resources.contains(resource)) {
            out_ << "_" << sep;
        } else [[likely]] {
            out_ << value << sep;
        }
    };

    print_resource(core::Resources::IRON, state.resources.iron_amount);
    print_resource(core::Resources::GOLD, state.resources.gold_amount);
    print_resource(core::Resources::GEM, state.resources.gems_amount);
    print_resource(core::Resources::EXPERIENCE, state.resources.experience_amount, "\n");
}

void BotBase::log_results() {
    auto state = game_->player_state();
    out_ << "result " << state.resources.iron_amount << " " << state.resources.gold_amount << " "
         << state.resources.gems_amount << " " << state.resources.experience_amount << " " << state.total_value << "\n";
}

void BotBase::log_move_action(uint8_t room) {
    out_ << "go " << static_cast<uint16_t>(room) << "\n";
}

void BotBase::log_harvest_action(core::ResourceType type) {
    switch (type) {
        case core::ResourceType::IRON:
            out_ << "collect iron\n";
            break;
        case core::ResourceType::GOLD:
            out_ << "collect gold\n";
            break;
        case core::ResourceType::GEM:
            out_ << "collect gems\n";
            break;
        case core::ResourceType::EXPERIENCE:
            out_ << "collect exp\n";
            break;
    }
}


const core::Game &BotBase::game() const {
    return *game_;
}

}  // namespace tvb::bot
