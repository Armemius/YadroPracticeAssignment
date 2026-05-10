#pragma once

#include "core/resources.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

namespace tvb::core {

class Player;

/// Class representing whole dungeon
class Dungeon final {
   public:
    /// Struct representing certain room in the dungeon
    class Room final {
       public:
        Room(uint8_t idx, std::unordered_map<Resource, uint8_t> resources, std::vector<uint8_t> adjacent_rooms);

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

        /**
         * @brief Returns info about available resources at the room
         * 
         * @return const std::unordered_map<Resource, uint8_t>& available resources
         */
        [[nodiscard]] const std::unordered_map<Resource, uint8_t> &resources() const noexcept;

       private:
        uint8_t idx_;                                      ///< Index of the room
        std::unordered_map<Resource, uint8_t> resources_;  ///< Current amount of resources in the room
        std::vector<uint8_t> adjacent_rooms_;              ///< Indices of rooms where you can get from current room

        friend class Dungeon;
    };

    struct RoomView {
        /// Index of the room
        std::optional<uint8_t> idx;

        ///Information about adjacent rooms
        std::optional<std::reference_wrapper<const std::vector<uint8_t>>> adjacent_rooms;

        /// Resources available at the room
        std::optional<std::reference_wrapper<const std::unordered_map<Resource, uint8_t>>> resources;
    };

    /**
     * @brief Returns room view by given index
     * 
     * @param room index of the room
     * @param player player to provide info to
     * @exception std::out_of_range throws if given room is not found
     * @return Room& lvalue reference to the room
     */
    [[nodiscard]] RoomView get_room(const Player &player, uint8_t room) const;

    /**
     * @brief Moves player to another room
     * 
     * @param player player to move
     * @param room target room
     * @exception std::logic_error throw if action is not permitted
     */
    void move(Player &player, uint8_t room);

    /**
     * @brief Harvests resources for the player
     * 
     * @param player player that should harvest resources
     * @param resource resource to harvest
     * @exception std::logic_error throw if action is not permitted
     */
    void harvest(Player &player, const Resource &resource);

    /**
     * @brief Construct a new Dungeon object
     * 
     * @param rooms map of the rooms in the dungeon
     */
    explicit Dungeon(std::unordered_map<uint8_t, Room> rooms);

   private:
    /**
     * @brief Updates player knowledge depending on the room
     * 
     * @param player Player to update knowledge
     * @param room Room the player is currently in
     */
    void update_knowledge(Player &player, uint8_t room) const;

    std::unordered_map<uint8_t, Room> rooms_;  ///< Map of rooms in the current dungeon, maps rooms to their indices
};

}  // namespace tvb::core
