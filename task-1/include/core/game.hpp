#pragma once

#include "core/dungeon.hpp"
#include "core/player.hpp"
#include "core/resources.hpp"

#include <cstdint>
#include <memory>
#include <unordered_set>

namespace tvb::core {

/// Facade class representing the game flow
class Game {
   private:
    std::unique_ptr<Player> player_;    ///< Current game's player
    std::unique_ptr<Dungeon> dungeon_;  ///< Dungeon in which the game takes place

   public:
    struct ResourcesState {
        uint16_t iron_amount;        ///< Amount of iron in certain place
        uint16_t gold_amount;        ///< Amount of gold in certain place
        uint16_t gems_amount;        ///< Amount of gems in certain place
        uint16_t experience_amount;  ///< Amount of experience in certain place
    };

    struct RoomState {
        uint16_t room_idx;                                 ///< Index of the room
        ResourcesState resources;                          ///< Resources present at the room
        std::unordered_set<Resource> harvested_resources;  ///< Harvested resources in that room
    };

    struct PlayerState {
        ResourcesState resources;  ///< Resources harvested by player
        uint32_t total_value;      ///< Total current value of resources
    };

    /**
    * @brief Construct a new Game object
    * 
    * @param player Player object
    * @param dungeon Dungeon object
    */
    Game(std::unique_ptr<Player> player, std::unique_ptr<Dungeon> dungeon);

    /**
     * @brief Returns amount of food available
     * 
     * @return uint16_t food left
     */
    [[nodiscard]] uint16_t player_food() const noexcept;

    /**
     * @brief Checks if player is alive
     * 
     * @return true player is alive
     * @return false player is dead
     */
    [[nodiscard]] bool player_alive() const noexcept;

    /**
     * @brief Returns index of the room player currently in
     * 
     * @return uint16_t index of the room
     */
    [[nodiscard]] uint16_t player_room() const noexcept;

    /**
     * @brief Returns total value of resources harvested
     * 
     * @return uint32_t total value of resources harvested
     */
    [[nodiscard]] uint32_t player_value() const noexcept;

    /**
     * @brief Returns amount of resource that player owns
     * 
     * @param resource resource to check
     * @return uint16_t amount of resource
     */
    [[nodiscard]] uint16_t player_amount(const Resource &resource) const;

    /**
     * @brief Returns state of the room player currently in
     * 
     * @exception std::out_of_range throws if given room is not found
     * @return RoomState Object representing room state
     */
    [[nodiscard]] RoomState player_room_state() const;

    /**
     * @brief Returns current state of the player
     * 
     * @return PlayerState Object representing player state
     */
    [[nodiscard]] PlayerState player_state() const noexcept;

    /**
     * @brief Returns room view by given index
     * 
     * @param room index of the room
     * @exception std::out_of_range throws if given room is not found
     * @return Dungeon::RoomView available information for the room
     */
    [[nodiscard]] Dungeon::RoomView get_room_info(uint8_t room) const;

    /**
     * @brief Get the current room object
     * 
     * @return const Dungeon::Room& current room object
     */
    [[nodiscard]] const Dungeon::Room &get_current_room() const;

    /**
     * @brief Get the room knowledge object
     * 
     * @return RoomKnowledge Player's knowledge about certain room
     */
    [[nodiscard]] RoomKnowledge get_room_knowledge(uint8_t room) const;

    /**
     * @brief Moves player to another room
     * 
     * @param player player to move
     * @param room target room
     * @exception std::logic_error throw if action is not permitted
     */
    void move_player(uint8_t room);

    /**
     * @brief Harvests resources for the player
     * 
     * @param player player that should harvest resources
     * @param resource resource to harvest
     * @exception std::logic_error throw if action is not permitted
     */
    void harvest(const Resource &resource);
};

}  // namespace tvb::core
