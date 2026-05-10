#pragma once

#include "core/resources.hpp"

#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace tvb::core {

enum class RoomKnowledge {
    UNKNOWN = 0,  ///< Room is unknown by the player
    KNOWN = 1,    ///< Room knows index of the room
    VISIBLE = 2,  ///< Player can see adjacent rooms to the current room
    VISITED = 3   //< Player can see all the information about the room
};

/// Class representing player knowledge of the dungeon
class PlayerKnowledge final {
   public:
    /**
     * @brief Check player's access to specified room
     * 
     * @param room index of the roomx
     * @return RoomKnowledge player's available knowledge about the room
     */
    [[nodiscard]] RoomKnowledge access(uint8_t room) const;

   private:
    /**
     * @brief Promotes knowledge of the room to specified level
     * 
     * @param room index of the room
     * @param level level of knowledge
     */
    void promote(uint8_t room, RoomKnowledge level);

    std::unordered_map<uint8_t, RoomKnowledge> accesses_;  ///< Rooms access for the player

    std::unordered_set<uint8_t> harvested_rooms_;  ///< Rooms where player has harvested resources

    friend class Dungeon;
};

/// Class representing player
class Player final {
   public:
    Player(ResourceType target_resource, uint8_t food);

    /**
     * @brief Returns amount of food available
     * 
     * @return uint8_t food left
     */
    [[nodiscard]] uint8_t food() const noexcept;

    /**
     * @brief Checks if player is alive
     * 
     * @return true player is alive
     * @return false player is dead
     */
    [[nodiscard]] bool alive() const noexcept;

    /**
     * @brief Returns index of the room player currently in
     * 
     * @return uint8_t index of the room
     */
    [[nodiscard]] uint8_t room() const noexcept;

    /**
     * @brief Returns total value of resources harvested
     * 
     * @return uint32_t
     */
    [[nodiscard]] uint32_t value() const noexcept;

    /**
     * @brief Returns amount of resource that player owns
     * 
     * @param resource resource to check
     * @return uint16_t amount of resource
     */
    [[nodiscard]] uint16_t amount(const Resource &resource) const;

   private:
    PlayerKnowledge knowledge_;                                   ///< State of the player's knowledge of the dungeon
    std::unordered_map<Resource, uint16_t> harvested_resources_;  ///< Resources harvested during current session
    ResourceType target_resource_{};                              ///< Resource that have value bonus
    uint8_t current_room_idx_{};                                  ///< Current player position represented by room index
    uint8_t food_left_{};                                         ///< Amount of the food left

    friend class Dungeon;
};

}  // namespace tvb::core
