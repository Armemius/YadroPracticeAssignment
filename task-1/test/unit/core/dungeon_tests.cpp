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

Dungeon make_dungeon_without_entrance() {
    std::unordered_map<uint8_t, Room> rooms;
    rooms.emplace(1, Room{1, {{Resources::GOLD, 1}}, {}});
    return Dungeon{std::move(rooms)};
}

Dungeon make_dungeon_with_missing_adjacent_room() {
    std::unordered_map<uint8_t, Room> rooms;
    rooms.emplace(0, Room{0, {}, {9}});
    return Dungeon{std::move(rooms)};
}

Dungeon make_dungeon_with_zero_quantity_resource() {
    std::unordered_map<uint8_t, Room> rooms;
    rooms.emplace(0, Room{0, {}, {1}});
    rooms.emplace(1, Room{1, {{Resources::GOLD, 0}, {Resources::IRON, 1}}, {0}});
    return Dungeon{std::move(rooms)};
}

Dungeon make_dungeon_with_inconsistent_adjacent_rooms() {
    std::unordered_map<uint8_t, Room> rooms;
    rooms.emplace(0, Room{0, {}, {2}});
    rooms.emplace(1, Room{1, {}, {0}});
    rooms.emplace(2, Room{1, {}, {3}});
    rooms.emplace(3, Room{1, {}, {0}});
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

TEST(DungeonTest, RoomReportsResourcePresenceCountsAndTopology) {
    Room room{7, {{Resources::IRON, 2}, {Resources::GOLD, 0}}, {1, 9, 3}};

    EXPECT_EQ(room.idx(), 7);
    EXPECT_TRUE(room.has(Resources::IRON));
    EXPECT_FALSE(room.has(Resources::GOLD));
    EXPECT_FALSE(room.has(Resources::GEM));
    EXPECT_EQ(room.count(Resources::IRON), 2);
    EXPECT_EQ(room.count(Resources::GOLD), 0);
    EXPECT_EQ(room.count(Resources::GEM), 0);
    EXPECT_EQ(room.adjacent_rooms(), (std::vector<uint8_t>{1, 3, 9}));
    EXPECT_EQ(room.resources().at(Resources::IRON), 2);
    EXPECT_TRUE(room.harvested_resources().empty());
}

TEST(DungeonTest, ExposesOnlyInformationAllowedByPlayerKnowledge) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    auto entrance = dungeon.get_available_room_info(player, 0);
    ASSERT_TRUE(entrance.idx.has_value());
    EXPECT_EQ(*entrance.idx, 0);
    EXPECT_TRUE(entrance.adjacent_rooms.has_value());
    EXPECT_TRUE(entrance.resources.has_value());

    dungeon.move(player, 1);

    auto visited = dungeon.get_available_room_info(player, 1);
    ASSERT_TRUE(visited.idx.has_value());
    EXPECT_TRUE(visited.adjacent_rooms.has_value());
    EXPECT_TRUE(visited.resources.has_value());

    auto visible = dungeon.get_available_room_info(player, 2);
    ASSERT_TRUE(visible.idx.has_value());
    EXPECT_TRUE(visible.adjacent_rooms.has_value());
    EXPECT_FALSE(visible.resources.has_value());

    auto known = dungeon.get_available_room_info(player, 3);
    ASSERT_TRUE(known.idx.has_value());
    EXPECT_FALSE(known.adjacent_rooms.has_value());
    EXPECT_FALSE(known.resources.has_value());
}

TEST(DungeonTest, UnknownExistingRoomHidesAllInformation) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    auto hidden = dungeon.get_available_room_info(player, 3);

    EXPECT_FALSE(hidden.idx.has_value());
    EXPECT_FALSE(hidden.adjacent_rooms.has_value());
    EXPECT_FALSE(hidden.resources.has_value());
}

TEST(DungeonTest, CurrentRoomIsFullyVisibleEvenBeforeKnowledgeWasUpdated) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    auto current = dungeon.get_available_room_info(player, player.room());

    ASSERT_TRUE(current.idx.has_value());
    EXPECT_EQ(*current.idx, 0);
    EXPECT_TRUE(current.adjacent_rooms.has_value());
    EXPECT_TRUE(current.resources.has_value());
}

TEST(DungeonTest, GetRoomThrowsForNonexistentRoom) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    EXPECT_THROW((void)dungeon.get_available_room_info(player, 99), std::out_of_range);
}

TEST(DungeonTest, MovesAcrossUnsortedAdjacentRoomsAndConsumesFood) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);

    EXPECT_EQ(player.room(), 1);
    EXPECT_EQ(player.food(), 5);
}

TEST(DungeonTest, MoveBackToEntranceWithLastFoodLeavesPlayerAliveAtZeroFood) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 2};

    dungeon.move(player, 1);
    dungeon.move(player, 0);

    EXPECT_EQ(player.room(), 0);
    EXPECT_EQ(player.food(), 0);
    EXPECT_TRUE(player.alive());
}

TEST(DungeonTest, RejectsImpossibleMoveWithoutConsumingFood) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    EXPECT_THROW(dungeon.move(player, 3), std::logic_error);
    EXPECT_EQ(player.room(), 0);
    EXPECT_EQ(player.food(), 6);
}

TEST(DungeonTest, RejectsMoveToNonexistentRoomWithoutConsumingFood) {
    EXPECT_THROW(make_dungeon_with_missing_adjacent_room(), std::out_of_range);
}

TEST(DungeonTest, RejectsMoveWhenCurrentRoomDoesNotExist) {
    auto dungeon = make_dungeon_without_entrance();
    Player player{ResourceType::GOLD, 6};

    EXPECT_THROW(dungeon.move(player, 1), std::logic_error);
    EXPECT_EQ(player.room(), 0);
    EXPECT_EQ(player.food(), 6);
}

TEST(DungeonTest, RejectsMoveWithNoFoodAtEntranceWithoutChangingState) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 0};

    EXPECT_TRUE(player.alive());
    EXPECT_THROW(dungeon.move(player, 1), std::logic_error);
    EXPECT_EQ(player.room(), 0);
    EXPECT_EQ(player.food(), 0);
}

TEST(DungeonTest, RejectsMoveForDeadPlayerOutsideEntrance) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 1};

    dungeon.move(player, 1);
    ASSERT_FALSE(player.alive());

    EXPECT_THROW(dungeon.move(player, 0), std::logic_error);
    EXPECT_EQ(player.room(), 1);
    EXPECT_EQ(player.food(), 0);
}

TEST(DungeonTest, HarvestCollectsAllResourcesAndChargesOnlyRepeatedRoomHarvests) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);
    dungeon.harvest(player, Resources::GOLD);

    auto first_harvest_view = dungeon.get_available_room_info(player, 1);
    ASSERT_TRUE(first_harvest_view.resources.has_value());
    EXPECT_EQ(first_harvest_view.resources->get().at(Resources::GOLD), 0);
    EXPECT_EQ(player.food(), 5);
    EXPECT_EQ(player.amount(Resources::GOLD), 2);

    dungeon.harvest(player, Resources::IRON);

    auto second_harvest_view = dungeon.get_available_room_info(player, 1);
    ASSERT_TRUE(second_harvest_view.resources.has_value());
    EXPECT_EQ(second_harvest_view.resources->get().at(Resources::IRON), 0);
    EXPECT_EQ(player.food(), 4);
    EXPECT_EQ(player.amount(Resources::IRON), 2);
}

TEST(DungeonTest, FirstHarvestIsFreeInEachDifferentRoom) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);
    dungeon.harvest(player, Resources::GOLD);
    EXPECT_EQ(player.food(), 5);

    dungeon.move(player, 2);
    dungeon.harvest(player, Resources::EXPERIENCE);
    EXPECT_EQ(player.food(), 4);
    EXPECT_EQ(player.amount(Resources::EXPERIENCE), 3);
}

TEST(DungeonTest, RepeatedHarvestInSameRoomCostsFoodEvenForDifferentResource) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);
    dungeon.harvest(player, Resources::GOLD);
    dungeon.harvest(player, Resources::IRON);

    EXPECT_EQ(player.food(), 4);
    EXPECT_EQ(player.amount(Resources::GOLD), 2);
    EXPECT_EQ(player.amount(Resources::IRON), 2);
}

TEST(DungeonTest, RepeatedRoomHarvestCanSpendLastFoodAndLeavePlayerDeadOutsideEntrance) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 2};

    dungeon.move(player, 1);
    dungeon.harvest(player, Resources::GOLD);
    dungeon.harvest(player, Resources::IRON);

    EXPECT_EQ(player.room(), 1);
    EXPECT_EQ(player.food(), 0);
    EXPECT_FALSE(player.alive());
    EXPECT_EQ(player.amount(Resources::GOLD), 2);
    EXPECT_EQ(player.amount(Resources::IRON), 2);
}

TEST(DungeonTest, RejectsHarvestForDeadPlayerOutsideEntranceWithoutChangingResources) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 1};

    dungeon.move(player, 1);
    ASSERT_FALSE(player.alive());

    EXPECT_THROW(dungeon.harvest(player, Resources::GOLD), std::logic_error);
    EXPECT_EQ(player.amount(Resources::GOLD), 0);

    auto room = dungeon.get_available_room_info(player, 1);
    ASSERT_TRUE(room.resources.has_value());
    EXPECT_EQ(room.resources->get().at(Resources::GOLD), 2);
}

TEST(DungeonTest, RejectsHarvestAtEntranceWithNoFoodWithoutChangingResources) {
    std::unordered_map<uint8_t, Room> rooms;
    rooms.emplace(0, Room{0, {{Resources::GOLD, 1}}, {}});
    Dungeon dungeon{std::move(rooms)};
    Player player{ResourceType::GOLD, 0};

    EXPECT_TRUE(player.alive());
    EXPECT_THROW(dungeon.harvest(player, Resources::GOLD), std::logic_error);
    EXPECT_EQ(player.amount(Resources::GOLD), 0);

    auto entrance = dungeon.get_available_room_info(player, 0);
    ASSERT_TRUE(entrance.resources.has_value());
    EXPECT_EQ(entrance.resources->get().at(Resources::GOLD), 1);
}

TEST(DungeonTest, RejectsHarvestWhenCurrentRoomDoesNotExist) {
    auto dungeon = make_dungeon_without_entrance();
    Player player{ResourceType::GOLD, 6};

    EXPECT_THROW(dungeon.harvest(player, Resources::GOLD), std::logic_error);
    EXPECT_EQ(player.amount(Resources::GOLD), 0);
    EXPECT_EQ(player.food(), 6);
}

TEST(DungeonTest, RejectsHarvestOfMissingResourceWithoutConsumingFood) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);

    EXPECT_THROW(dungeon.harvest(player, Resources::EXPERIENCE), std::logic_error);
    EXPECT_EQ(player.food(), 5);
    EXPECT_EQ(player.amount(Resources::EXPERIENCE), 0);
}

TEST(DungeonTest, RejectsHarvestOfZeroQuantityResourceWithoutConsumingFood) {
    auto dungeon = make_dungeon_with_zero_quantity_resource();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);

    EXPECT_THROW(dungeon.harvest(player, Resources::GOLD), std::logic_error);
    EXPECT_EQ(player.food(), 5);
    EXPECT_EQ(player.amount(Resources::GOLD), 0);

    auto room = dungeon.get_available_room_info(player, 1);
    ASSERT_TRUE(room.resources.has_value());
    EXPECT_EQ(room.resources->get().at(Resources::GOLD), 0);
}

TEST(DungeonTest, RejectsHarvestAfterResourceIsExhaustedWithoutExtraFoodCost) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);
    dungeon.harvest(player, Resources::GOLD);
    ASSERT_EQ(player.food(), 5);

    EXPECT_THROW(dungeon.harvest(player, Resources::GOLD), std::logic_error);
    EXPECT_EQ(player.food(), 5);
    EXPECT_EQ(player.amount(Resources::GOLD), 2);

    auto room = dungeon.get_available_room_info(player, 1);
    ASSERT_TRUE(room.resources.has_value());
    EXPECT_EQ(room.resources->get().at(Resources::GOLD), 0);
}

TEST(DungeonTest, ProvidesFullInfoAboutExistentCurrentRoom) {
    auto dungeon = make_test_dungeon();
    Player player{ResourceType::GOLD, 6};

    dungeon.move(player, 1);
    const auto &room = dungeon.get_curent_room_info(player);
    EXPECT_EQ(room.idx(), 1);
    EXPECT_EQ(room.count(Resources::GOLD), 2);
    EXPECT_FALSE(room.has(Resources::EXPERIENCE));
}

TEST(DungeonTest, ThrowsOnNonexistentCurrentRoom) {
    auto dungeon = make_dungeon_without_entrance();
    Player player{ResourceType::GOLD, 6};

    EXPECT_THROW((void)dungeon.get_curent_room_info(player), std::logic_error);
}

TEST(DungeonTest, AdjacentRoomsInfoIsConsistentBetweenRooms) {
    auto dungeon = make_dungeon_with_inconsistent_adjacent_rooms();
    Player player{ResourceType::GOLD, 6};

    {
        const auto &entrance_room = dungeon.get_curent_room_info(player);
        EXPECT_EQ(entrance_room.adjacent_rooms(), (std::vector<uint8_t>{1, 2, 3}));
    }

    dungeon.move(player, 2);

    {
        const auto &entrance_room = dungeon.get_curent_room_info(player);
        EXPECT_EQ(entrance_room.adjacent_rooms(), (std::vector<uint8_t>{0, 3}));
    }
}

TEST(DungeonTest, ProvidesCorrectPlayerKnowledgeLevel) {
    auto dungeon = make_big_test_dungeon();
    Player player{ResourceType::GOLD, 6};
    dungeon.init_player(player);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 0), RoomKnowledge::VISITED);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 1), RoomKnowledge::VISIBLE);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 2), RoomKnowledge::VISIBLE);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 3), RoomKnowledge::VISIBLE);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 4), RoomKnowledge::KNOWN);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 5), RoomKnowledge::UNKNOWN);
    dungeon.move(player, 1);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 0), RoomKnowledge::VISITED);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 1), RoomKnowledge::VISITED);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 2), RoomKnowledge::VISIBLE);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 3), RoomKnowledge::VISIBLE);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 4), RoomKnowledge::VISIBLE);
    EXPECT_EQ(dungeon.get_room_knowledge(player, 5), RoomKnowledge::KNOWN);
}

}  // namespace
}  // namespace tvb::core
