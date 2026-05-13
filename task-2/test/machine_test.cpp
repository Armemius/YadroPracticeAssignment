#include <gtest/gtest.h>

#include "simulation/machine.hpp"
#include "simulation/types.hpp"

#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace sim {
namespace {

Machine create_machine(machine_t index) {
    std::vector<product_t> products{{.index = 0, .type = 0}, {.index = 1, .type = 1}, {.index = 2, .type = 2}};
    std::unordered_map<product_type_t, optime_t> optimes{{0, 5}, {1, 7}, {2, 9}};
    return Machine{index, std::move(products), std::move(optimes)};
}

Machine create_empty_machine(machine_t index) {
    std::vector<product_t> products{};
    std::unordered_map<product_type_t, optime_t> optimes{{0, 5}, {1, 7}, {2, 9}};
    return Machine{index, std::move(products), std::move(optimes)};
}

Machine create_fast_machine(machine_t index) {
    std::vector<product_t> products{{.index = 0, .type = 0}, {.index = 1, .type = 1}, {.index = 2, .type = 2}};
    std::unordered_map<product_type_t, optime_t> optimes{{0, 0}, {1, 0}, {2, 0}};
    return Machine{index, std::move(products), std::move(optimes)};
}

TEST(MachineTests, DoNotStartProcessingAutomatically) {
    Machine machine = create_machine(42);
    EXPECT_FALSE(machine.processing());
    EXPECT_FALSE(machine.ready());
    EXPECT_TRUE(machine.idle());
}

TEST(MachineTests, RejectsTickingBackwardsInTime) {
    Machine machine = create_machine(42);
    EXPECT_NO_THROW(machine.tick(1));
    EXPECT_THROW(machine.tick(1), std::logic_error);
    EXPECT_THROW(machine.tick(0), std::logic_error);
    EXPECT_NO_THROW(machine.tick(2));
}

TEST(MachineTests, ProducesCorrectWaitingTimes) {
    Machine machine = create_machine(42);
    EXPECT_EQ(machine.queue_time(), 21);
    EXPECT_NE(machine.current_processing_time(), 5);
    machine.start();
    EXPECT_EQ(machine.queue_time(), 21);
    EXPECT_EQ(machine.current_processing_time(), 5);
}

TEST(MachineTests, ShiftsWaitingTimesOnNoop) {
    Machine machine = create_machine(42);
    EXPECT_EQ(machine.queue_time(), 21);
    machine.tick(10);
    EXPECT_EQ(machine.queue_time(), 31);
}

TEST(MachineTests, ShiftsWaitingTimesOnNoopWhenSomeOperationIsInProgress) {
    Machine machine = create_machine(42);
    machine.start();
    EXPECT_EQ(machine.queue_time(), 21);
    machine.tick(100);
    EXPECT_EQ(machine.queue_time(), 116);
}

TEST(MachineTests, UpdatesTimeOnEnqueue) {
    Machine machine = create_machine(42);
    EXPECT_EQ(machine.queue_time(), 21);
    EXPECT_EQ(machine.enqueue({.index = 42, .type = 0}), 26);
    EXPECT_EQ(machine.queue_time(), 26);
}

TEST(MachineTests, ProducesCorrectQueuedWaitTime) {
    Machine machine = create_machine(42);
    EXPECT_EQ(machine.wait_time(), 21);
}

TEST(MachineTests, DecreasesQueuedWaitTimeOnStart) {
    Machine machine = create_machine(42);
    EXPECT_FALSE(machine.start());
    EXPECT_EQ(machine.wait_time(), 16);

    machine.tick(5);
    (void)machine.yield();
    EXPECT_FALSE(machine.start());
    EXPECT_EQ(machine.wait_time(), 9);
}

TEST(MachineTests, UpdatesQueuedWaitTimeOnEnqueue) {
    Machine machine = create_machine(42);
    EXPECT_EQ(machine.wait_time(), 21);

    machine.enqueue({.index = 42, .type = 0});
    EXPECT_EQ(machine.wait_time(), 26);

    machine.enqueue({.index = 43, .type = 2});
    EXPECT_EQ(machine.wait_time(), 35);
}

TEST(MachineTests, DoesNotShiftQueuedWaitTimeWhenIdleTimePasses) {
    Machine machine = create_machine(42);
    EXPECT_EQ(machine.wait_time(), 21);

    machine.tick(10);
    EXPECT_EQ(machine.queue_time(), 31);
    EXPECT_EQ(machine.wait_time(), 21);
}

TEST(MachineTests, DoesNotIncludeImmediatelyStartedProductInQueuedWaitTime) {
    Machine machine = create_empty_machine(42);
    EXPECT_EQ(machine.wait_time(), 0);

    EXPECT_FALSE(machine.start({.index = 42, .type = 1}));
    EXPECT_EQ(machine.wait_time(), 0);
}

TEST(MachineTests, ProvidesCorrectInfoAboutReadyState) {
    Machine machine = create_machine(42);
    machine.start();
    EXPECT_FALSE(machine.idle());
    EXPECT_TRUE(machine.processing());
    EXPECT_FALSE(machine.ready());
    machine.tick(4);
    EXPECT_FALSE(machine.idle());
    EXPECT_TRUE(machine.processing());
    EXPECT_FALSE(machine.ready());
    machine.tick(8);
    EXPECT_TRUE(machine.idle());
    EXPECT_FALSE(machine.processing());
    EXPECT_TRUE(machine.ready());
}

TEST(MachineTests, ProvidesCorrectInfoAboutReadyStateInTickOperations) {
    Machine machine = create_machine(42);
    machine.start();
    EXPECT_FALSE(machine.tick(4));
    EXPECT_TRUE(machine.tick(5));
    EXPECT_TRUE(machine.tick(6));
}

TEST(MachineTests, ProvidesCorrectInfoAboutReadyStateInStartOperations) {
    {
        Machine machine = create_machine(42);
        EXPECT_FALSE(machine.start());
    }
    {
        Machine machine = create_fast_machine(42);
        EXPECT_TRUE(machine.start());
    }
}

TEST(MachineTests, UpgradesItemTypeAfterProcessing) {
    {
        Machine machine = create_machine(42);
        machine.start();
        machine.tick(100);
        product_t product = machine.yield();
        EXPECT_EQ(product.type, 1);
    }
    {
        Machine machine = create_fast_machine(42);
        machine.start();
        product_t product = machine.yield();
        EXPECT_EQ(product.type, 1);
    }
}

TEST(MachineTests, FollowsItemProcessingOrder) {
    Machine machine = create_fast_machine(42);
    machine.enqueue({.index = 10, .type = 0});
    machine.enqueue({.index = 11, .type = 1});
    machine.enqueue({.index = 12, .type = 2});
    std::vector<product_type_t> results;
    results.reserve(6);
    while (machine.queue_size() > 0) {
        machine.start();
        product_t product = machine.yield();
        results.push_back(product.type);
    }
    EXPECT_EQ(results, (std::vector<product_type_t>{1, 2, 3, 1, 2, 3}));
}

TEST(MachineTests, RejectsStartOfProcessingWhileResultIsNotYielded) {
    Machine machine = create_fast_machine(42);
    machine.start();
    EXPECT_THROW(machine.start(), std::logic_error);
}

TEST(MachineTests, RejectsStartOfProcessingWhileOtherProcessing) {
    Machine machine = create_machine(42);
    machine.start();
    machine.tick(4);
    EXPECT_THROW(machine.start(), std::logic_error);
}

TEST(MachineTests, RejectsStartOfProcessingWhenNoItemsAvaialbe) {
    Machine machine = create_empty_machine(42);
    EXPECT_THROW(machine.start(), std::out_of_range);
}

TEST(MachineTests, RejectYieldWhenNoOutputAvailable) {
    Machine machine = create_machine(42);
    EXPECT_THROW(machine.yield(), std::out_of_range);
}

TEST(MachineTests, AllowsToProcessItemImmidiately) {
    Machine machine = create_empty_machine(42);
    EXPECT_NO_THROW(machine.start({0, 0}));
}

TEST(MachineTests, RejectsToStartImmediateProcessingWhenQueueIsNotEmpty) {
    Machine machine = create_machine(42);
    EXPECT_THROW(machine.start({0, 0}), std::logic_error);
}

TEST(MachineTests, CorrectlyShowsProcessedItems) {
    Machine machine = create_machine(42);
    machine.start();
    EXPECT_EQ(machine.current_processing(), (product_t{.index = 0, .type = 0}));
}

TEST(MachineTests, CorrectlyShowsNextItesmToProcess) {
    Machine machine = create_machine(42);
    EXPECT_EQ(machine.next_item(), (product_t{.index = 0, .type = 0}));
    machine.start();
    EXPECT_EQ(machine.next_item(), (product_t{.index = 1, .type = 1}));
}

TEST(MachineTests, RejectsShowingProcessedItemIfNoAvailable) {
    Machine machine = create_machine(42);
    EXPECT_THROW((void)machine.current_processing(), std::out_of_range);
}

TEST(MachineTests, RejectsShowingNextProcessedItemIfNoAvailable) {
    Machine machine = create_empty_machine(42);
    EXPECT_THROW((void)machine.next_item(), std::out_of_range);
}

}  // namespace
}  // namespace sim
