#pragma once

#include <cstdint>
#include "bot/bot_base.hpp"

namespace tvb::bot {

class OriginalBot final : public BotBase {
   public:
    OriginalBot(std::shared_ptr<core::Game> game, std::ostream &out);

   protected:
    void next() override;

    [[nodiscard]] bool finished() const override;

   private:
    uint8_t food_threshold_{};  ///< Food threshold to switch state
};
}  // namespace tvb::bot
