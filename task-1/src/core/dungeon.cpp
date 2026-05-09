#include "core/dungeon.hpp"
#include <stdexcept>

namespace tvb::core {

Dungeon::Room &Dungeon::operator[](uint8_t index) {
    return rooms_.at(index);
}

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

}  // namespace tvb::core
