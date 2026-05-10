#include "core/game.hpp"
#include "core/resources.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
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

Dungeon make_big_test_dungeon() {
    std::unordered_map<uint8_t, Room> rooms;
    rooms.emplace(0, Room{0, {}, {1, 2, 3}});
    rooms.emplace(1, Room{1, {}, {0, 4, 2}});
    rooms.emplace(2, Room{2, {}, {0, 1}});
    rooms.emplace(3, Room{3, {}, {0, 4}});
    rooms.emplace(4, Room{4, {}, {1, 3, 5}});
    rooms.emplace(5, Room{5, {}, {4}});
    return Dungeon{std::move(rooms)};
}

std::unique_ptr<Dungeon> make_test_dungeon_ptr() {
    return std::make_unique<Dungeon>(make_test_dungeon());
}

std::unique_ptr<Dungeon> make_big_test_dungeon_ptr() {
    return std::make_unique<Dungeon>(make_big_test_dungeon());
}

std::unique_ptr<Player> make_player_ptr(ResourceType target_resource = ResourceType::GOLD, uint8_t food = 6) {
    return std::make_unique<Player>(target_resource, food);
}

Game make_game(ResourceType target_resource = ResourceType::GOLD, uint8_t food = 6) {
    return Game{make_player_ptr(target_resource, food), make_test_dungeon_ptr()};
}

Game make_big_game(ResourceType target_resource = ResourceType::GOLD, uint8_t food = 6) {
    return Game{make_player_ptr(target_resource, food), make_big_test_dungeon_ptr()};
}

TEST(GameTest, RejectsNullPlayerOrDungeon) {
    EXPECT_THROW(Game(nullptr, make_test_dungeon_ptr()), std::invalid_argument);
    EXPECT_THROW(Game(make_player_ptr(), nullptr), std::invalid_argument);
}

TEST(GameTest, ExposesInitialPlayerAndRoomState) {
    auto game = make_game(ResourceType::GOLD, 6);

    EXPECT_EQ(game.player_food(), 6);
    EXPECT_TRUE(game.player_alive());
    EXPECT_EQ(game.player_room(), 0);
    EXPECT_EQ(game.player_value(), 0);
    EXPECT_EQ(game.player_amount(Resources::GOLD), 0);

    auto player_state = game.player_state();
    EXPECT_EQ(player_state.resources.iron_amount, 0);
    EXPECT_EQ(player_state.resources.gold_amount, 0);
    EXPECT_EQ(player_state.resources.gems_amount, 0);
    EXPECT_EQ(player_state.resources.experience_amount, 0);
    EXPECT_EQ(player_state.total_value, 0);

    auto room_state = game.player_room_state();
    EXPECT_EQ(room_state.room_idx, 0);
    EXPECT_EQ(room_state.resources.iron_amount, 0);
    EXPECT_EQ(room_state.resources.gold_amount, 0);
    EXPECT_EQ(room_state.resources.gems_amount, 0);
    EXPECT_EQ(room_state.resources.experience_amount, 0);
    EXPECT_TRUE(room_state.harvested_resources.empty());
}

TEST(GameTest, UsedResourcesAreStoredPerRoom) {
    auto game = make_game(ResourceType::GOLD, 6);

    game.move_player(1);
    game.harvest(Resources::GOLD);
    ASSERT_TRUE(game.player_room_state().harvested_resources.contains(Resources::GOLD));

    game.move_player(2);

    EXPECT_EQ(game.player_room(), 2);
    EXPECT_EQ(game.player_food(), 4);
    EXPECT_TRUE(game.player_room_state().harvested_resources.empty());

    game.move_player(1);
    EXPECT_TRUE(game.player_room_state().harvested_resources.contains(Resources::GOLD));
}

TEST(GameTest, HarvestUpdatesRoomStatePlayerStateAndUsedResources) {
    auto game = make_game(ResourceType::GOLD, 6);

    game.move_player(1);
    game.harvest(Resources::GOLD);

    auto room_state = game.player_room_state();
    EXPECT_EQ(room_state.room_idx, 1);
    EXPECT_EQ(room_state.resources.iron_amount, 2);
    EXPECT_EQ(room_state.resources.gold_amount, 0);
    EXPECT_EQ(room_state.resources.gems_amount, 1);
    EXPECT_EQ(room_state.resources.experience_amount, 0);
    EXPECT_EQ(room_state.harvested_resources.size(), 1);
    EXPECT_TRUE(room_state.harvested_resources.contains(Resources::GOLD));

    auto player_state = game.player_state();
    EXPECT_EQ(player_state.resources.iron_amount, 0);
    EXPECT_EQ(player_state.resources.gold_amount, 2);
    EXPECT_EQ(player_state.resources.gems_amount, 0);
    EXPECT_EQ(player_state.resources.experience_amount, 0);
    EXPECT_EQ(player_state.total_value, 22);
    EXPECT_EQ(game.player_amount(Resources::GOLD), 2);
    EXPECT_EQ(game.player_value(), 22);
}

TEST(GameTest, RepeatedHarvestUsesCoreFoodAndScoringRules) {
    auto game = make_game(ResourceType::GEM, 10);

    game.move_player(1);
    game.harvest(Resources::GOLD);
    game.harvest(Resources::IRON);
    game.harvest(Resources::GEM);

    EXPECT_EQ(game.player_food(), 7);

    auto player_state = game.player_state();
    EXPECT_EQ(player_state.resources.iron_amount, 2);
    EXPECT_EQ(player_state.resources.gold_amount, 2);
    EXPECT_EQ(player_state.resources.gems_amount, 1);
    EXPECT_EQ(player_state.resources.experience_amount, 0);
    EXPECT_EQ(player_state.total_value, 43);

    auto room_state = game.player_room_state();
    EXPECT_EQ(room_state.harvested_resources.size(), 3);
    EXPECT_TRUE(room_state.harvested_resources.contains(Resources::GOLD));
    EXPECT_TRUE(room_state.harvested_resources.contains(Resources::IRON));
    EXPECT_TRUE(room_state.harvested_resources.contains(Resources::GEM));
    EXPECT_EQ(room_state.resources.iron_amount, 0);
    EXPECT_EQ(room_state.resources.gold_amount, 0);
    EXPECT_EQ(room_state.resources.gems_amount, 0);
}

TEST(GameTest, GetRoomInfoUsesPlayerKnowledge) {
    auto game = make_game(ResourceType::GOLD, 6);

    auto hidden = game.get_room_info(3);
    EXPECT_FALSE(hidden.idx.has_value());
    EXPECT_FALSE(hidden.adjacent_rooms.has_value());
    EXPECT_FALSE(hidden.resources.has_value());

    game.move_player(1);

    auto visible = game.get_room_info(2);
    ASSERT_TRUE(visible.idx.has_value());
    EXPECT_EQ(*visible.idx, 2);
    EXPECT_TRUE(visible.adjacent_rooms.has_value());
    EXPECT_FALSE(visible.resources.has_value());

    auto known = game.get_room_info(3);
    ASSERT_TRUE(known.idx.has_value());
    EXPECT_EQ(*known.idx, 3);
    EXPECT_FALSE(known.adjacent_rooms.has_value());
    EXPECT_FALSE(known.resources.has_value());
}

TEST(GameTest, PropagatesMoveAndHarvestErrorsWithoutChangingGameState) {
    auto game = make_game(ResourceType::GOLD, 6);

    EXPECT_THROW(game.move_player(3), std::logic_error);
    EXPECT_EQ(game.player_room(), 0);
    EXPECT_EQ(game.player_food(), 6);
    EXPECT_TRUE(game.player_room_state().harvested_resources.empty());

    game.move_player(1);
    game.harvest(Resources::GOLD);
    ASSERT_TRUE(game.player_room_state().harvested_resources.contains(Resources::GOLD));

    EXPECT_THROW(game.harvest(Resources::EXPERIENCE), std::logic_error);
    auto room_state = game.player_room_state();
    EXPECT_EQ(room_state.harvested_resources.size(), 1);
    EXPECT_TRUE(room_state.harvested_resources.contains(Resources::GOLD));
    EXPECT_EQ(room_state.resources.gold_amount, 0);
    EXPECT_EQ(game.player_amount(Resources::EXPERIENCE), 0);
}

TEST(GameTest, PropagatesCurrentRoomLookupErrors) {
    std::unordered_map<uint8_t, Room> rooms;
    rooms.emplace(1, Room{1, {{Resources::GOLD, 1}}, {}});
    auto game = Game{make_player_ptr(), std::make_unique<Dungeon>(std::move(rooms))};

    EXPECT_THROW((void)game.player_room_state(), std::logic_error);
}

TEST(GameTest, ProvidesCorrectPlayerKnowledgeLevel) {
    auto game = make_big_game();
    EXPECT_EQ(game.get_room_knowledge(0), RoomKnowledge::VISITED);
    EXPECT_EQ(game.get_room_knowledge(1), RoomKnowledge::VISIBLE);
    EXPECT_EQ(game.get_room_knowledge(2), RoomKnowledge::VISIBLE);
    EXPECT_EQ(game.get_room_knowledge(3), RoomKnowledge::VISIBLE);
    EXPECT_EQ(game.get_room_knowledge(4), RoomKnowledge::KNOWN);
    EXPECT_EQ(game.get_room_knowledge(5), RoomKnowledge::UNKNOWN);
    game.move_player(1);
    EXPECT_EQ(game.get_room_knowledge(0), RoomKnowledge::VISITED);
    EXPECT_EQ(game.get_room_knowledge(1), RoomKnowledge::VISITED);
    EXPECT_EQ(game.get_room_knowledge(2), RoomKnowledge::VISIBLE);
    EXPECT_EQ(game.get_room_knowledge(3), RoomKnowledge::VISIBLE);
    EXPECT_EQ(game.get_room_knowledge(4), RoomKnowledge::VISIBLE);
    EXPECT_EQ(game.get_room_knowledge(5), RoomKnowledge::KNOWN);
}

}  // namespace
}  // namespace tvb::core
