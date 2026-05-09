#pragma once

#include "core/resources.hpp"

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tvb::core {

class Player;

/// Class representing whole dungeon
class Dungeon final {
   public:
    /// Struct representing certain room in the dungeon
    class Room final {
       public:
        Room(uint8_t idx, std::unordered_map<Resource, uint8_t> resources, std::vector<uint8_t> adjacent_rooms)
            : idx_(idx), resources_(std::move(resources)), adjacent_rooms_(std::move(adjacent_rooms)) {}

        /**
         * @brief Returns index of the room
         * 
         * @return uint8_t room index
         */
        [[nodiscard]] uint8_t idx() const;

        /**
         * @brief Checks if resource is available in the room
         * 
         * @param resource resource to check
         * @return true resource is present
         * @return false no available resources
         */
        [[nodiscard]] bool has(const Resource &resource) const;

        /**
         * @brief Counts available resource in the room
         * 
         * @param resource resource to check
         * @return int amount of resource available
         */
        [[nodiscard]] int count(const Resource &resource) const;

        /**
         * @brief Returns indices of rooms adjacent to this one
         * 
         * @return const std::vector<uint8_t>& adjacent rooms
         */
        [[nodiscard]] const std::vector<uint8_t> &adjacent_rooms() const noexcept;

       private:
        bool resources_harvested_{false};                  ///< Show if resources were already harvested in the room
        uint8_t idx_;                                      ///< Index of the room
        std::unordered_map<Resource, uint8_t> resources_;  ///< Current amount of resources in the room
        std::vector<uint8_t> adjacent_rooms_;              ///< Indices of rooms where you can get from current room
    };

    /**
     * @brief Returns room by given index
     * 
     * @param index index of the room
     * @exception std::out_of_range throws if given room is not found
     * @return Room& lvalue reference to the room
     */
    [[nodiscard]] Room &operator[](uint8_t index);

    /**
     * @brief Construct a new Dungeon object
     * 
     * @param rooms map of the rooms in the dungeon
     */
    explicit Dungeon(std::unordered_map<uint8_t, Room> rooms) : rooms_(std::move(rooms)) {}

   private:
    std::unordered_map<uint8_t, Room> rooms_;  ///< Map of rooms in the current dungeon, maps rooms to their indices
};

}  // namespace tvb::core
