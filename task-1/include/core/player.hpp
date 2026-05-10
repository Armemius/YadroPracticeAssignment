#pragma once

#include "core/resources.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace tvb::core {

enum class RoomKnowledge {
    UNKNOWN = 0,  ///< Room is unknown by the player
    KNOWN = 1,    ///< Room knows index of the room
    VISIBLE = 2,  ///< Player can see adjacent rooms to the current room
    VISITED = 3   //< Player can see all the information about the room
};

/// Iterator wrapper for ranges of rooms representing visibility levels
class RoomIndexIterator {
   public:
    using value_type = uint8_t;
    using reference = uint8_t;
    using pointer = void;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    RoomIndexIterator() = default;

    /**
     * @brief Construct a new Room Index Iterator object
     *
     * @param it iterator for the knowledge set
     */
    explicit RoomIndexIterator(std::set<std::pair<RoomKnowledge, uint8_t>>::iterator it);

    uint8_t operator*() const;

    RoomIndexIterator &operator++();

    RoomIndexIterator operator++(int);

    RoomIndexIterator &operator--();

    RoomIndexIterator operator--(int);

    friend bool operator==(const RoomIndexIterator &lhs, const RoomIndexIterator &rhs);

    friend bool operator!=(const RoomIndexIterator &lhs, const RoomIndexIterator &rhs);

   private:
    std::set<std::pair<RoomKnowledge, uint8_t>>::iterator iterator_;
};

static_assert(std::bidirectional_iterator<RoomIndexIterator>);

/// Iterator that merges two sorted room-index ranges into one sorted room-index stream
class MergedRoomIndexIterator {
   public:
    using value_type = uint8_t;
    using reference = uint8_t;
    using pointer = void;
    using difference_type = ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    MergedRoomIndexIterator() = default;

    MergedRoomIndexIterator(RoomIndexIterator first_begin, RoomIndexIterator first_end, RoomIndexIterator second_begin,
                            RoomIndexIterator second_end);

    uint8_t operator*() const;

    MergedRoomIndexIterator &operator++();

    MergedRoomIndexIterator operator++(int);

    friend bool operator==(const MergedRoomIndexIterator &lhs, const MergedRoomIndexIterator &rhs);

    friend bool operator!=(const MergedRoomIndexIterator &lhs, const MergedRoomIndexIterator &rhs);

   private:
    [[nodiscard]] bool should_take_first() const;

    RoomIndexIterator first_;
    RoomIndexIterator first_end_;
    RoomIndexIterator second_;
    RoomIndexIterator second_end_;
};

static_assert(std::input_iterator<MergedRoomIndexIterator>);

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

    /**
     * @brief Checks if player had already harvested certain room
     * 
     * @param room Room to check
     * @return true If the room was already harvested
     * @return false If the room was not harvested yet
     */
    [[nodiscard]] bool harvested(uint8_t room) const;

    /**
     * @brief Returns range of known rooms for the player
     *
     * @return std::pair<RoomIndexIterator, RoomIndexIterator> Begin and end iterators for the range
     */
    [[nodiscard]] std::pair<RoomIndexIterator, RoomIndexIterator> known_rooms() const;

    /**
     * @brief Returns range of visible rooms for the player
     *
     * @return std::pair<RoomIndexIterator, RoomIndexIterator> Begin and end iterators for the range
     */
    [[nodiscard]] std::pair<RoomIndexIterator, RoomIndexIterator> visible_rooms() const;

    /**
     * @brief Returns range of visited rooms for the player
     *
     * @return std::pair<RoomIndexIterator, RoomIndexIterator> Begin and end iterators for the range
     */
    [[nodiscard]] std::pair<RoomIndexIterator, RoomIndexIterator> visited_rooms() const;

    /**
     * @brief Returns range of nonvisited rooms for the player
     *
     * @return std::pair<MergedRoomIndexIterator, MergedRoomIndexIterator> Begin and end iterators for the range
     */
    [[nodiscard]] std::pair<MergedRoomIndexIterator, MergedRoomIndexIterator> nonvisited_rooms() const;

   private:
    /**
     * @brief Promotes knowledge of the room to specified level
     * 
     * @param room index of the room
     * @param level level of knowledge
     */
    void promote(uint8_t room, RoomKnowledge level);

    /// Indicies for faster checks for specific rooms
    std::unordered_map<uint8_t, std::set<std::pair<RoomKnowledge, uint8_t>>::iterator> knowledge_indices_;

    /// Ordered storage for room knowledge
    std::set<std::pair<RoomKnowledge, uint8_t>> accesses_;

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
