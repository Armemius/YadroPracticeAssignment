#pragma once

#include "bot/bot_base.hpp"

#include <cstdint>
#include <queue>
#include <vector>

namespace tvb::bot {

class OriginalBot final : public BotBase {
   public:
    OriginalBot(std::shared_ptr<core::Game> game, std::ostream &out);

   protected:
    /**
     * @brief Process next step for the original bot algo
     *
     */
    void next() override;

    [[nodiscard]] bool finished() const override;

   private:
    /**
     * @brief Process next step for exploration phase
     *
     */
    void next_explore();

    /**
     * @brief Process next step for backtracking
     *
     */
    void next_backtrack();

    /**
     * @brief Try to harvest best free resource if possible
     * 
     * @return true Resource was harvested
     * @return false Cannot harvest resource
     */
    [[nodiscard]] bool try_harvest_best_free_resource();

    /**
     * @brief Try to harvest best extra resource
     * 
     * @return true Resource was harvested
     * @return false Cannot harvest resource
     */
    [[nodiscard]] bool try_harvest_best_extra_resource();

    /**
     * @brief Get best available resource at current room
     * 
     * @return const core::Resource* best resource to harvest or nullptr if none
     */
    [[nodiscard]] const core::Resource *best_available_resource() const;

    /**
     * @brief Calculates shortest path to the next unvisited room
     * 
     * @return std::vector<uint8_t> Path to the room
     */
    [[nodiscard]] std::vector<uint8_t> path_to_next_unvisited() const;

    /**
     * @brief Calculates shortest path to the entrance room
     * 
     * @return std::vector<uint8_t> Path to the entrance
     */
    [[nodiscard]] std::vector<uint8_t> path_to_entrance() const;

    /**
     * @brief Calculates shortest path to unvisited room
     * 
     * @param target_room Room to find path into
     * @return std::vector<uint8_t> Path to the room
     */
    [[nodiscard]] std::vector<uint8_t> path_to_unvisited(uint8_t target_room) const;

    /**
     * @brief Calculates shortest path to the room
     * 
     * @param target_room Room to find path into
     * @param target_may_be_unvisited Specifies if the target room can be unvisited
     * @return std::vector<uint8_t> Path to the room
     */
    [[nodiscard]] std::vector<uint8_t> shortest_path(uint8_t target_room, bool target_may_be_unvisited) const;

    /**
     * @brief Set the current bot path
     * 
     * @param path New bot's path
     */
    void set_current_path(const std::vector<uint8_t> &path);

    uint8_t food_threshold_{};          ///< Minimum food amount preserved for returning
    bool returning_{};                  ///< Whether bot has switched to returning phase
    std::queue<uint8_t> current_path_;  ///< Current bot's path
};
}  // namespace tvb::bot
