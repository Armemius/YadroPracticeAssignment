#include "core/player.hpp"

#include <cstdint>
#include <limits>
#include <numeric>
#include <type_traits>

namespace tvb::core {

RoomIndexIterator::RoomIndexIterator(std::set<std::pair<RoomKnowledge, uint8_t>>::iterator it) : iterator_(it) {}

uint8_t RoomIndexIterator::operator*() const {
    return iterator_->second;
}

RoomIndexIterator &RoomIndexIterator::operator++() {
    ++iterator_;
    return *this;
}

RoomIndexIterator RoomIndexIterator::operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
}

RoomIndexIterator &RoomIndexIterator::operator--() {
    --iterator_;
    return *this;
}

RoomIndexIterator RoomIndexIterator::operator--(int) {
    auto tmp = *this;
    --(*this);
    return tmp;
}

bool operator==(const RoomIndexIterator &lhs, const RoomIndexIterator &rhs) {
    return lhs.iterator_ == rhs.iterator_;
}

bool operator!=(const RoomIndexIterator &lhs, const RoomIndexIterator &rhs) {
    return !(lhs == rhs);
}

MergedRoomIndexIterator::MergedRoomIndexIterator(RoomIndexIterator first_begin, RoomIndexIterator first_end,
                                                 RoomIndexIterator second_begin, RoomIndexIterator second_end)
    : first_(first_begin), first_end_(first_end), second_(second_begin), second_end_(second_end) {}

uint8_t MergedRoomIndexIterator::operator*() const {
    if (first_ == first_end_) {
        return *second_;
    }
    if (second_ == second_end_) {
        return *first_;
    }
    return should_take_first() ? *first_ : *second_;
}

MergedRoomIndexIterator &MergedRoomIndexIterator::operator++() {
    if (first_ == first_end_ && second_ == second_end_) {
        return *this;
    }
    if (first_ != first_end_ && second_ != second_end_ && *first_ == *second_) {
        ++first_;
        ++second_;
        return *this;
    }
    if (should_take_first()) {
        ++first_;
    } else {
        ++second_;
    }
    return *this;
}

MergedRoomIndexIterator MergedRoomIndexIterator::operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
}

bool MergedRoomIndexIterator::should_take_first() const {
    if (second_ == second_end_) {
        return true;
    }
    if (first_ == first_end_) {
        return false;
    }
    return *first_ < *second_;
}

bool operator==(const MergedRoomIndexIterator &lhs, const MergedRoomIndexIterator &rhs) {
    return lhs.first_ == rhs.first_ && lhs.first_end_ == rhs.first_end_ && lhs.second_ == rhs.second_ &&
           lhs.second_end_ == rhs.second_end_;
}

bool operator!=(const MergedRoomIndexIterator &lhs, const MergedRoomIndexIterator &rhs) {
    return !(lhs == rhs);
}

RoomKnowledge PlayerKnowledge::access(uint8_t room) const {
    if (!knowledge_indices_.contains(room)) {
        return RoomKnowledge::UNKNOWN;
    }
    return knowledge_indices_.at(room)->first;
}

bool PlayerKnowledge::harvested(uint8_t room) const {
    return harvested_rooms_.contains(room);
}

void PlayerKnowledge::promote(uint8_t room, RoomKnowledge level) {
    if (!knowledge_indices_.contains(room)) {
        auto [pos, _] = accesses_.insert({level, room});
        knowledge_indices_.emplace(room, pos);
        return;
    }

    auto current_level = static_cast<std::underlying_type_t<RoomKnowledge>>(knowledge_indices_.at(room)->first);
    auto target_level = static_cast<std::underlying_type_t<RoomKnowledge>>(level);

    if (current_level < target_level) {
        auto pos = knowledge_indices_.at(room);
        auto node = accesses_.extract(pos);
        node.value().first = level;
        knowledge_indices_.at(room) = accesses_.insert(std::move(node)).position;
    }
}

std::pair<RoomIndexIterator, RoomIndexIterator> PlayerKnowledge::known_rooms() const {
    auto start = accesses_.lower_bound({RoomKnowledge::KNOWN, std::numeric_limits<uint8_t>::min()});
    auto end = accesses_.upper_bound({RoomKnowledge::KNOWN, std::numeric_limits<uint8_t>::max()});

    return {RoomIndexIterator{start}, RoomIndexIterator{end}};
}

std::pair<RoomIndexIterator, RoomIndexIterator> PlayerKnowledge::visible_rooms() const {
    auto start = accesses_.lower_bound({RoomKnowledge::VISIBLE, std::numeric_limits<uint8_t>::min()});
    auto end = accesses_.upper_bound({RoomKnowledge::VISIBLE, std::numeric_limits<uint8_t>::max()});

    return {RoomIndexIterator{start}, RoomIndexIterator{end}};
}

std::pair<RoomIndexIterator, RoomIndexIterator> PlayerKnowledge::visited_rooms() const {
    auto start = accesses_.lower_bound({RoomKnowledge::VISITED, std::numeric_limits<uint8_t>::min()});
    auto end = accesses_.upper_bound({RoomKnowledge::VISITED, std::numeric_limits<uint8_t>::max()});

    return {RoomIndexIterator{start}, RoomIndexIterator{end}};
}

std::pair<MergedRoomIndexIterator, MergedRoomIndexIterator> PlayerKnowledge::nonvisited_rooms() const {
    auto [known_begin, known_end] = known_rooms();
    auto [visible_begin, visible_end] = visible_rooms();

    return {MergedRoomIndexIterator{known_begin, known_end, visible_begin, visible_end},
            MergedRoomIndexIterator{known_end, known_end, visible_end, visible_end}};
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
    return std::accumulate(
        harvested_resources_.cbegin(), harvested_resources_.cend(), 0U,
        [&](auto acc, const auto &current) { return acc + (current.second * resource_value(current.first)); });
}

uint16_t Player::amount(const Resource &resource) const {
    if (!harvested_resources_.contains(resource)) {
        return 0;
    }
    return harvested_resources_.at(resource);
}

uint16_t Player::resource_value(const Resource &resource) const noexcept {
    if (resource.type() == target_resource_) {
        return resource.value();
    }
    return (resource.value() / 2) + (resource.value() % 2);
}

}  // namespace tvb::core
