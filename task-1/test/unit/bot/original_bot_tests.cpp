#include <gtest/gtest.h>

#include "bot/original_bot.hpp"
#include "parser/parser.hpp"

#include <memory>
#include <sstream>

namespace tvb::bot {
namespace {

std::string run_bot(const char *input_text) {
    std::istringstream input{input_text};
    std::ostringstream output;
    auto game = std::make_shared<core::Game>(parser::parse(input));
    OriginalBot bot{game, output};

    bot.run();

    return output.str();
}

TEST(OriginalBotTest, ProducesTaskSampleOutput) {
    constexpr const char *INPUT = R"(5
0 1,2 0 0 0 0
1 0,3 5 2 1 15
2 0,4 3 2 1 10
3 1,4 1 0 2 40
4 2,5 2 4 0 15
5 4 0 5 4 10
6 gems
)";

    constexpr const char *EXPECTED = R"(go 1
state 1 5 2 1 15
collect gems
state 1 5 2 _ 15
go 3
state 3 1 0 2 40
collect gems
state 3 1 0 _ 40
go 4
state 4 2 4 0 15
collect gold
state 4 2 _ 0 15
go 3
state 3 1 0 _ 40
go 1
state 1 5 2 _ 15
go 0
result 0 4 3 0 93
)";

    EXPECT_EQ(run_bot(INPUT), EXPECTED);
}

TEST(OriginalBotTest, PassingThroughEntranceDuringExplorationDoesNotFinishRun) {
    constexpr const char *INPUT = R"(2
0 1,2 0 0 0 0
1 0 0 1 0 0
2 0 0 0 1 0
6 gems
)";

    constexpr const char *EXPECTED = R"(go 1
state 1 0 1 0 0
collect gold
state 1 0 _ 0 0
go 0
state 0 0 0 0 0
go 2
state 2 0 0 1 0
collect gems
state 2 0 0 _ 0
go 0
result 0 1 1 0 29
)";

    EXPECT_EQ(run_bot(INPUT), EXPECTED);
}

}  // namespace
}  // namespace tvb::bot
