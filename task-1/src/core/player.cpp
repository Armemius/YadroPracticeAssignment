#include "core/player.hpp"

#include <cstdint>
#include <numeric>
#include <type_traits>

namespace tvb::core {

RoomKnowledge PlayerKnowledge::access(uint8_t room) const {
    if (!accesses_.contains(room)) {
        return RoomKnowledge::UNKNOWN;
    }
    return accesses_.at(room);
}

void PlayerKnowledge::promote(uint8_t room, RoomKnowledge level) {
    if (!accesses_.contains(room)) {
        accesses_.emplace(room, level);
        return;
    }

    auto current_level = static_cast<std::underlying_type_t<RoomKnowledge>>(accesses_.at(room));
    auto target_level = static_cast<std::underlying_type_t<RoomKnowledge>>(level);

    if (current_level < target_level) {
        accesses_.at(room) = level;
    }
}

Player::Player(ResourceType target_resource, uint8_t food) : target_resource_(target_resource), food_left_(food) {}

uint8_t Player::food() const noexcept {
    return food_left_;
}

bool Player::alive() const noexcept {
    return food_left_ > 0 || current_room_idx_ == 0;
}

uint8_t Player::room() const noexcept {
    return current_room_idx_;
}

uint32_t Player::value() const noexcept {
    return std::accumulate(harvested_resources_.cbegin(), harvested_resources_.cend(), 0U,
                           [&](auto acc, const auto &current) {
                               if (current.first.type() == target_resource_) {
                                   return acc + (current.second * current.first.value() * 2);
                               }
                               return acc + (current.second * current.first.value());
                           });
}

uint16_t Player::amount(const Resource &resource) const {
    if (!harvested_resources_.contains(resource)) {
        return 0;
    }
    return harvested_resources_.at(resource);
}

}  // namespace tvb::core
