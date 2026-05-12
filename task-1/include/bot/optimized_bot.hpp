#pragma once

#include "bot/bot_base.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace tvb::bot {

/// Bot implementation using dynamic return control and room-value heuristics.
class OptimizedBot final : public BotBase {
   public:
    /**
     * @brief Construct a new Optimized Bot object
     *
     * @param game Game for bot to play
     * @param out Logs output stream
     */
    OptimizedBot(std::shared_ptr<core::Game> game, std::ostream &out);

   protected:
    /**
     * @brief Process the next optimized bot action
     *
     */
    void next() override;

    /**
     * @brief Checks if bot has finished execution
     *
     * @return true Bot returned to the entrance or explicitly finished
     * @return false Game is still in progress
     */
    [[nodiscard]] bool finished() const override;

   private:
    /// Candidate route with data used by the return-route heuristic.
    struct Route {
        std::vector<uint8_t> rooms;   ///< Ordered rooms to move through
        uint16_t nonvisited_rooms{};  ///< Number of not-yet-visited rooms on the route
        uint32_t expected_value{};    ///< Expected value available along the route
    };

    /**
     * @brief Process next step for exploration mode
     *
     */
    void next_explore();

    /**
     * @brief Process next step for returning to the entrance
     *
     */
    void next_return();

    /**
     * @brief Store current room value on first visit for unknown-room estimation
     *
     */
    void remember_current_room_value();

    /**
     * @brief Finish execution and print final results at the entrance
     *
     */
    void finish_at_entrance();

    /**
     * @brief Decide if bot must switch from exploration to returning mode
     *
     * @return true Food is only enough for the known return path or no exploration remains
     * @return false Exploration can continue
     */
    [[nodiscard]] bool should_return() const;

    /**
     * @brief Try to harvest the best resource when current room harvest is free
     *
     * @return true Resource was harvested
     * @return false No free harvest or no resources are available
     */
    [[nodiscard]] bool try_harvest_best_free_resource();

    /**
     * @brief Try to harvest the best extra resource without breaking return safety
     *
     * @return true Resource was harvested
     * @return false Extra harvest is unsafe or no resources are available
     */
    [[nodiscard]] bool try_harvest_best_extra_resource();

    /**
     * @brief Returns best available resource at current room
     *
     * @return const core::Resource* best resource to harvest or nullptr if none
     */
    [[nodiscard]] const core::Resource *best_available_resource() const;

    /**
     * @brief Choose the best adjacent non-visited room for immediate exploration
     *
     * @return std::optional<uint8_t> next room index or std::nullopt if none is suitable
     */
    [[nodiscard]] std::optional<uint8_t> best_adjacent_exploration_step() const;

    /**
     * @brief Find the best known path to a non-visited room
     *
     * @return std::optional<std::vector<uint8_t>> path to follow or std::nullopt if none is reachable
     */
    [[nodiscard]] std::optional<std::vector<uint8_t>> best_exploration_path() const;

    /**
     * @brief Find the best route back to the entrance
     *
     * @return std::optional<Route> return route or std::nullopt if no route is known
     */
    [[nodiscard]] std::optional<Route> best_return_route() const;

    /**
     * @brief Calculates shortest path from current room to target room
     *
     * @param target_room Room to find path into
     * @return std::optional<std::vector<uint8_t>> path to target or std::nullopt if unreachable
     */
    [[nodiscard]] std::optional<std::vector<uint8_t>> shortest_path(uint8_t target_room) const;

    /**
     * @brief Calculates shortest path between two accessible rooms
     *
     * @param source_room Room to start from
     * @param target_room Room to find path into
     * @return std::optional<std::vector<uint8_t>> path to target or std::nullopt if unreachable
     */
    [[nodiscard]] std::optional<std::vector<uint8_t>> shortest_path_from(uint8_t source_room,
                                                                         uint8_t target_room) const;

    /**
     * @brief Returns adjacent rooms if adjacency is known
     *
     * @param room Room to inspect
     * @return std::optional<std::vector<uint8_t>> adjacent rooms or std::nullopt if unknown
     */
    [[nodiscard]] std::optional<std::vector<uint8_t>> adjacent_rooms(uint8_t room) const;

    /**
     * @brief Checks if room is known enough to be used in path search
     *
     * @param room Room to check
     * @return true Room is not unknown or is the current room
     * @return false Room cannot be used yet
     */
    [[nodiscard]] bool can_use_room(uint8_t room) const;

    /**
     * @brief Checks if moving to room preserves a known or estimated return path
     *
     * @param room Room to move into
     * @return true Move is safe according to current knowledge
     * @return false Move may leave bot without enough food to return
     */
    [[nodiscard]] bool can_safely_move_to(uint8_t room) const;

    /**
     * @brief Calculates heuristic score for exploring a room
     *
     * @param room Room to score
     * @param movement_cost Food needed to reach the room
     * @return double Room score
     */
    [[nodiscard]] double exploration_score(uint8_t room, uint16_t movement_cost) const;

    /**
     * @brief Calculates propagated exploration bonus for a room
     *
     * @param room Room to score
     * @param depth Current propagation depth
     * @return double Exploration bonus
     */
    [[nodiscard]] double exploration_bonus(uint8_t room, uint8_t depth) const;

    /**
     * @brief Estimates room value for exploration heuristics
     *
     * @param room Room to estimate
     * @return uint32_t Expected room value
     */
    [[nodiscard]] uint32_t expected_room_value(uint8_t room) const;

    /**
     * @brief Calculates known resource value for a room
     *
     * @param room Room to inspect
     * @return uint32_t Known resource value or zero if resources are unknown
     */
    [[nodiscard]] uint32_t known_room_value(uint8_t room) const;

    /**
     * @brief Calculates value of resources currently visible in player's room
     *
     * @return uint32_t Current room resource value
     */
    [[nodiscard]] uint32_t current_room_value() const;

    /**
     * @brief Calculates average first-visit room value
     *
     * @return uint32_t Average value used for unknown-room estimates
     */
    [[nodiscard]] uint32_t average_first_visit_value() const;

    /**
     * @brief Calculates shortest known return distance to the entrance
     *
     * @return uint16_t Return distance or max value if no route is known
     */
    [[nodiscard]] uint16_t return_distance() const;

    bool returning_{};  ///< Whether bot has switched to returning mode
    bool finished_{};   ///< Whether bot explicitly finished and logged result

    std::unordered_map<uint8_t, uint32_t> first_visit_values_;  ///< Room values observed on first visits
};

}  // namespace tvb::bot
