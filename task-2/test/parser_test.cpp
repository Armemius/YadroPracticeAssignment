#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <utility>

#define private public
#include "parser/parser.hpp"
#undef private

namespace parser {
namespace {

sim::Simulation parse_from(std::string input, std::ostringstream &output) {
    std::istringstream stream(std::move(input));
    return parse(stream, output);
}

void expect_invalid_line(std::string input, const std::string &line) {
    std::ostringstream output;
    std::istringstream stream(std::move(input));

    try {
        (void)parse(stream, output);
        FAIL() << "Expected InvalidInputLine";
    } catch (const InvalidInputLine &error) {
        EXPECT_EQ(error.line(), line);
    }
}

TEST(ParserTests, ParsesTaskSampleIntoReadySimulation) {
    std::ostringstream output;
    sim::Simulation simulation = parse_from(
        "3 2\n"
        "3 5\n"
        "4 6\n"
        "3 0 1 0\n"
        "2 1 0\n",
        output);

    EXPECT_EQ(simulation.product_type_count_, 3);
    EXPECT_EQ(simulation.tick_, 0);
    ASSERT_EQ(simulation.machines_.size(), 2);
    EXPECT_EQ(simulation.machines_[0].index(), 0);
    EXPECT_EQ(simulation.machines_[1].index(), 1);
    EXPECT_EQ(simulation.machines_[0].queue_size(), 3);
    EXPECT_EQ(simulation.machines_[1].queue_size(), 2);
    EXPECT_EQ(simulation.machines_[0].wait_time(), 10);
    EXPECT_EQ(simulation.machines_[1].wait_time(), 11);
    EXPECT_EQ(simulation.machines_[0].next_item(), (sim::product_t{.index = 0, .type = 0}));
    EXPECT_EQ(simulation.machines_[1].next_item(), (sim::product_t{.index = 3, .type = 1}));
}

TEST(ParserTests, ParsedSimulationCanRun) {
    std::ostringstream output;
    sim::Simulation simulation = parse_from(
        "2 1\n"
        "3\n"
        "2 0 0\n",
        output);

    simulation.run();

    EXPECT_TRUE(simulation.finished());
    EXPECT_EQ(simulation.tick_, 6);
}

TEST(ParserTests, AllowsNoOperationRowsWhenThereIsOnlyReadyType) {
    std::ostringstream output;
    sim::Simulation simulation = parse_from(
        "1 2\n"
        "0\n"
        "0\n",
        output);

    EXPECT_EQ(simulation.product_type_count_, 1);
    ASSERT_EQ(simulation.machines_.size(), 2);
    EXPECT_TRUE(simulation.finished());
}

TEST(ParserTests, InvalidInputLineStoresOriginalLine) {
    InvalidInputLine error{"bad line"};

    EXPECT_EQ(error.line(), "bad line");
    EXPECT_THROW(throw std::move(error), ParserError);
}

TEST(ParserTests, RejectsMissingHeader) {
    expect_invalid_line("", "");
}

TEST(ParserTests, RejectsHeaderWithWrongValueCount) {
    expect_invalid_line("3\n", "3");
}

TEST(ParserTests, RejectsUnreadableHeaderValue) {
    expect_invalid_line("3 x\n", "3 x");
}

TEST(ParserTests, RejectsOutOfRangeHeaderValue) {
    expect_invalid_line(
        "101 1\n"
        "0\n",
        "101 1");
}

TEST(ParserTests, RejectsOperationLineWithWrongValueCount) {
    expect_invalid_line(
        "3 2\n"
        "3\n",
        "3");
}

TEST(ParserTests, RejectsOperationTimeOutOfRange) {
    expect_invalid_line(
        "2 1\n"
        "10001\n"
        "0\n",
        "10001");
}

TEST(ParserTests, RejectsMissingQueueLine) {
    expect_invalid_line(
        "2 1\n"
        "1\n",
        "");
}

TEST(ParserTests, RejectsQueueLineWithWrongValueCount) {
    expect_invalid_line(
        "2 1\n"
        "1\n"
        "2 0\n",
        "2 0");
}

TEST(ParserTests, RejectsProductTypeOutOfRange) {
    expect_invalid_line(
        "2 1\n"
        "1\n"
        "1 1\n",
        "1 1");
}

TEST(ParserTests, RejectsProductWhenThereIsOnlyReadyType) {
    expect_invalid_line(
        "1 1\n"
        "1 0\n",
        "1 0");
}

TEST(ParserTests, RejectsTrailingNonEmptyLine) {
    expect_invalid_line(
        "2 1\n"
        "1\n"
        "0\n"
        "extra\n",
        "extra");
}

}  // namespace
}  // namespace parser
