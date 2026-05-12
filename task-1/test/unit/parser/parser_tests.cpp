#include <gtest/gtest.h>

#include "core/resources.hpp"
#include "parser/parser.hpp"

#include <sstream>

namespace tvb::parser {
namespace {

constexpr const char *VALID_INPUT = R"(5
0 1,2 0 0 0 0
1 0,3 5 2 1 15
2 0,4 3 2 1 10
3 1,4 1 0 2 40
4 2,5 2 4 0 15
5 4 0 5 4 10
6 gems
)";

TEST(ParserTest, ParsesValidGameInput) {
    std::istringstream input{VALID_INPUT};

    auto game = parse(input);

    EXPECT_EQ(game.player_food(), 6);
    EXPECT_EQ(game.player_room(), 0);
    EXPECT_EQ(game.player_value(), 0);

    auto entrance = game.player_room_state();
    EXPECT_EQ(entrance.room_idx, 0);
    EXPECT_EQ(entrance.resources.iron_amount, 0);
    EXPECT_EQ(entrance.resources.gold_amount, 0);
    EXPECT_EQ(entrance.resources.gems_amount, 0);
    EXPECT_EQ(entrance.resources.experience_amount, 0);

    game.move_player(1);
    auto first_room = game.player_room_state();
    EXPECT_EQ(first_room.room_idx, 1);
    EXPECT_EQ(first_room.resources.iron_amount, 5);
    EXPECT_EQ(first_room.resources.gold_amount, 2);
    EXPECT_EQ(first_room.resources.gems_amount, 1);
    EXPECT_EQ(first_room.resources.experience_amount, 15);

    game.harvest(core::Resources::GEM);
    EXPECT_EQ(game.player_value(), 23);
}

TEST(ParserTest, AllowsEntranceWithoutResourceColumns) {
    std::istringstream input{R"(1
0 1
1 0 1 2 3 4
2 iron
)"};

    auto game = parse(input);

    EXPECT_EQ(game.player_food(), 2);
    EXPECT_EQ(game.player_room_state().room_idx, 0);
    game.move_player(1);
    EXPECT_EQ(game.player_room_state().resources.iron_amount, 1);
}

TEST(ParserTest, RejectsInvalidSampleLine) {
    std::istringstream input{R"(5
0 1,2
1 0,3 5 2 1 15
2 0,4 3 2 1 10
3 1,4 1 0 2 40
4 2|5 2 4 0 15
5 4 0 5 4 10
6 gems
)"};

    try {
        (void)parse(input);
        FAIL() << "Expected InvalidInputLine";
    } catch (const InvalidInputLine &error) {
        EXPECT_STREQ(error.what(), "4 2|5 2 4 0 15");
        EXPECT_EQ(error.line(), "4 2|5 2 4 0 15");
    }
}

TEST(ParserTest, RejectsOutOfRangeAndMalformedValues) {
    std::istringstream input{R"(1
0 1 0 0 0 0
1 0 256 2 3 4
2 gold
)"};

    EXPECT_THROW((void)parse(input), InvalidInputLine);
}

TEST(ParserTest, RejectsDuplicateRoomIndexWithCurrentLine) {
    std::istringstream input{R"(1
0 1
0 1 1 2 3 4
2 exp
)"};

    try {
        (void)parse(input);
        FAIL() << "Expected InvalidInputLine";
    } catch (const InvalidInputLine &error) {
        EXPECT_EQ(error.line(), "0 1 1 2 3 4");
    }
}

TEST(ParserTest, RejectsUnknownTargetResource) {
    std::istringstream input{R"(1
0 1
1 0 1 2 3 4
2 wood
)"};

    try {
        (void)parse(input);
        FAIL() << "Expected InvalidInputLine";
    } catch (const InvalidInputLine &error) {
        EXPECT_EQ(error.line(), "2 wood");
    }
}

}  // namespace
}  // namespace tvb::parser
