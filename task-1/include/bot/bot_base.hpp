#pragma once

#include <cstdint>
#include <memory>
#include <ostream>
#include "core/game.hpp"
#include "core/resources.hpp"

namespace tvb::bot {

/// Base class for all bot algorithm implementations
class BotBase {
   public:
    /**
     * @brief Construct a new Bot Base object
     * 
     * @param game Game for bot to play
     * @param out Logs output stream
     */
    BotBase(std::shared_ptr<core::Game> game, std::ostream &out);

    /**
     * @brief Runs the bot till the end condition
     * 
     */
    void run();

   protected:
    /**
     * @brief Processes next move for the bot
     * 
     */
    virtual void next() = 0;

    /**
     * @brief Checks if bot has finished execution
     * 
     * @return true Bot is ready
     * @return false Game is still intact
     */
    [[nodiscard]] virtual bool finished() const = 0;

    /**
     * @brief Move to the certain room
     * 
     * @throw std::logic_error Impossible move
     * @param room room to move into
     */
    void move(uint8_t room);

    /**
     * @brief Harvest certain resource
     * 
     * @throw std::logic_error Impossible action
     * @param resource resource to harvest
     */
    void harvest(const core::Resource &resource);

    /**
     * @brief Prints current room state to the log stream
     * 
     */
    void log_room_state();

    /**
     * @brief Prints player state
     * 
     */
    void log_results();

    /**
     * @brief Prints movement action
     * 
     * @param room room to move into
     */
    void log_move_action(uint8_t room);

    /**
     * @brief Prints collection action
     * 
     * @param type Type of resource to harvest
     */
    void log_harvest_action(core::ResourceType type);

    /**
     * @brief Returns current game state
     * 
     * @return const core::Game& current game state
     */
    const core::Game &game() const;

   private:
    std::ostream &out_;                 ///< Logs output stream
    std::shared_ptr<core::Game> game_;  ///< Game state
};

}  // namespace tvb::bot
