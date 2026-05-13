#include <gtest/gtest.h>

#include "core/dungeon.hpp"
#include "core/player.hpp"
#include "core/resources.hpp"

#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tvb::core {
namespace {

using Room = Dungeon::Room;

Dungeon make_test_dungeon() {
    std::unordered_map<uint8_t, Room> rooms;
    rooms.emplace(0, Room{0, {}, {2, 1}});
    rooms.emplace(1, Room{1, {{Resources::IRON, 2}, {Resources::GOLD, 2}, {Resources::GEM, 1}}, {0, 2}});
    rooms.emplace(2, Room{2, {{Resources::EXPERIENCE, 3}}, {0, 1, 3}});
    rooms.emplace(3, Room{3, {{Resources::GOLD, 1}}, {2}});
    return Dungeon{std::move(rooms)};
}

TEST(PlayerTest, StartsAtEntranceWithRequestedFood) {
    Player player{ResourceType::GOLD, 6};

    EXPECT_EQ(player.room(), 0);
    EXPECT_EQ(player.food(), 6);
    EXPECT_TRUE(player.alive());
    EXPECT_EQ(player.value(), 0);
    EXPECT_EQ(player.amount(Resources::GOLD), 0);
}

TEST(PlayerTest, StartsAliveAtEntranceEvenWithNoFood) {
    Player player{ResourceType::GOLD, 0};

    EXPECT_EQ(player.room(), 0);
    EXPECT_EQ(player.food(), 0);
    EXPECT_TRUE(player.alive());
    EXPECT_EQ(player.value(), 0);
}

TEST(PlayerTest, StoresMaximumFoodValue) {
    Player player{ResourceType::EXPERIENCE, 255};

    EXPECT_EQ(player.food(), 255);
    EXPECT_TRUE(player.alive());
}

TEST(PlayerTest, BecomesDeadWithNoFoodOutsideEntrance) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 1};

    dungeon.move(player, 1);

    EXPECT_EQ(player.room(), 1);
    EXPECT_EQ(player.food(), 0);
    EXPECT_FALSE(player.alive());
}

TEST(PlayerTest, IsAliveAtEntranceEvenAfterUsingLastFoodToReturn) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 2};

    dungeon.move(player, 1);
    dungeon.move(player, 0);

    EXPECT_EQ(player.room(), 0);
    EXPECT_EQ(player.food(), 0);
    EXPECT_TRUE(player.alive());
}

TEST(PlayerTest, TracksHarvestedAmountsAndScore) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);
    dungeon.harvest(player, Resources::GOLD);
    dungeon.harvest(player, Resources::IRON);

    EXPECT_EQ(player.amount(Resources::GOLD), 2);
    EXPECT_EQ(player.amount(Resources::IRON), 2);
    EXPECT_EQ(player.amount(Resources::GEM), 0);
    EXPECT_EQ(player.value(), 30);
}

TEST(PlayerTest, ScoresTargetResourceAtFullValueAndOthersAtRoundedHalfValue) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GEM, 10};

    dungeon.move(player, 1);
    dungeon.harvest(player, Resources::GOLD);
    dungeon.harvest(player, Resources::IRON);
    dungeon.harvest(player, Resources::GEM);

    EXPECT_EQ(player.amount(Resources::GOLD), 2);
    EXPECT_EQ(player.amount(Resources::IRON), 2);
    EXPECT_EQ(player.amount(Resources::GEM), 1);
    EXPECT_EQ(player.amount(Resources::EXPERIENCE), 0);
    EXPECT_EQ(player.value(), 43);
}

TEST(PlayerTest, ScoresExperienceAsTargetResource) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::EXPERIENCE, 6};

    dungeon.move(player, 2);
    dungeon.harvest(player, Resources::EXPERIENCE);

    EXPECT_EQ(player.amount(Resources::EXPERIENCE), 3);
    EXPECT_EQ(player.value(), 3);
}

TEST(PlayerTest, FailedHarvestDoesNotChangeAmountOrScore) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);
    EXPECT_THROW(dungeon.harvest(player, Resources::EXPERIENCE), std::logic_error);

    EXPECT_EQ(player.amount(Resources::EXPERIENCE), 0);
    EXPECT_EQ(player.value(), 0);
}

}  // namespace
}  // namespace tvb::core
