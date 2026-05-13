#include <gtest/gtest.h>

#define private public
#define protected public
#include "simulation/machine.hpp"
#include "simulation/simulation.hpp"
#include "simulation/types.hpp"
#undef protected
#undef private

#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sim {
namespace {

Machine make_machine(machine_t index, const std::vector<product_t> &products, std::vector<optime_t> operation_times) {
    std::unordered_map<product_type_t, optime_t> optimes;
    for (product_type_t operation = 0; static_cast<size_t>(operation) < operation_times.size(); ++operation) {
        optimes.emplace(operation, operation_times[operation]);
    }
    return Machine{index, products, std::move(optimes)};
}

TEST(SimulationTests, RunProducesExpectedOutputForTaskSample) {
    std::ostringstream output;
    Simulation simulation{
        {make_machine(0, {{.index = 0, .type = 0}, {.index = 1, .type = 1}, {.index = 2, .type = 0}}, {3, 4}),
         make_machine(1, {{.index = 3, .type = 1}, {.index = 4, .type = 0}}, {5, 6})},
        3,
        output};

    simulation.run();

    EXPECT_EQ(output.str(),
              "start 0 0 0 0\n"
              "start 0 3 1 1\n"
              "finish 3 0 0 0\n"
              "start 3 1 1 0\n"
              "wait 3 0 1 1 1\n"
              "finish 6 3 1 1\n"
              "start 6 4 0 1\n"
              "ready 6 3 1\n"
              "finish 7 1 1 0\n"
              "start 7 2 0 0\n"
              "ready 7 1 0\n"
              "finish 10 2 0 0\n"
              "start 10 2 1 0\n"
              "finish 11 4 0 1\n"
              "start 11 0 1 1\n"
              "wait 11 4 1 0 0\n"
              "finish 14 2 1 0\n"
              "start 14 4 1 0\n"
              "ready 14 2 0\n"
              "finish 17 0 1 1\n"
              "ready 17 0 1\n"
              "finish 18 4 1 0\n"
              "ready 18 4 0\n"
              "stop 18\n");
}

TEST(SimulationTests, NextProcessesOneEventTimeAtOnce) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}}, {3})}, 2, output};

    simulation.next();
    EXPECT_EQ(output.str(), "start 0 0 0 0\n");

    simulation.next();
    EXPECT_EQ(output.str(),
              "start 0 0 0 0\n"
              "finish 3 0 0 0\n"
              "ready 3 0 0\n");
}

TEST(SimulationTests, RunStopsImmediatelyWhenThereAreNoProducts) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {}, {1})}, 2, output};

    simulation.run();

    EXPECT_EQ(output.str(), "stop 0\n");
}

TEST(SimulationTests, OrdersSameTickEventsAsFinishStartWaitReady) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}, {.index = 1, .type = 0}}, {1, 1}),
                           make_machine(1, {{.index = 2, .type = 1}, {.index = 3, .type = 0}}, {1, 1})},
                          3,
                          output};

    simulation.next();
    simulation.next();

    EXPECT_EQ(output.str(),
              "start 0 0 0 0\n"
              "start 0 2 1 1\n"
              "finish 1 0 0 0\n"
              "finish 1 2 1 1\n"
              "start 1 1 0 0\n"
              "start 1 3 0 1\n"
              "wait 1 0 1 0 0\n"
              "ready 1 2 1\n");
}

TEST(SimulationTests, InstantlyProcessedProductsCascadeAtTheSameTime) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}}, {0, 0})}, 3, output};

    simulation.run();

    EXPECT_EQ(output.str(),
              "start 0 0 0 0\n"
              "finish 0 0 0 0\n"
              "start 0 0 1 0\n"
              "finish 0 0 1 0\n"
              "ready 0 0 0\n"
              "stop 0\n");
}

TEST(SimulationTests, SelectsLowestIndexedMachineWhenQueueWaitTimesTie) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {}, {1, 1}), make_machine(1, {{.index = 0, .type = 0}}, {1, 1})}, 3, output};

    simulation.run();

    EXPECT_EQ(output.str(),
              "start 0 0 0 1\n"
              "finish 1 0 0 1\n"
              "start 1 0 1 0\n"
              "finish 2 0 1 0\n"
              "ready 2 0 0\n"
              "stop 2\n");
}

TEST(SimulationBlackBoxTests, ConstructorStoresInitialStateWithoutStartingWork) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}}, {2}), make_machine(1, {}, {3})}, 2, output};

    EXPECT_EQ(simulation.log_, &output);
    EXPECT_EQ(simulation.tick_, 0);
    EXPECT_EQ(simulation.product_type_count_, 2);
    ASSERT_EQ(simulation.machines_.size(), 2);
    EXPECT_EQ(simulation.machines_[0].queue_size(), 1);
    EXPECT_EQ(simulation.machines_[1].queue_size(), 0);
    EXPECT_TRUE(simulation.machines_[0].idle());
    EXPECT_TRUE(simulation.machines_[1].idle());
    EXPECT_EQ(simulation.wait_index_.size(), 2);
    EXPECT_EQ(*simulation.wait_index_.begin(), (std::pair<simtime_t, machine_t>{0, 1}));
    EXPECT_TRUE(simulation.finish_events_.empty());
    EXPECT_EQ(simulation.machine_versions_, (std::vector<uint64_t>{0, 0}));
}

TEST(SimulationBlackBoxTests, FinishedReturnsTrueOnlyWhenEveryMachineHasNoWork) {
    std::ostringstream output;
    Simulation empty_simulation{{make_machine(0, {}, {2}), make_machine(1, {}, {3})}, 2, output};
    EXPECT_TRUE(empty_simulation.finished());

    Simulation queued_simulation{{make_machine(0, {{.index = 0, .type = 0}}, {2})}, 2, output};
    EXPECT_FALSE(queued_simulation.finished());

    Machine ready_machine = make_machine(0, {}, {0});
    ASSERT_TRUE(ready_machine.start({.index = 0, .type = 0}));
    Simulation ready_simulation{{std::move(ready_machine)}, 2, output};
    EXPECT_FALSE(ready_simulation.finished());
}

TEST(SimulationBlackBoxTests, FinishedReturnsFalseWhileMachineIsProcessing) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}}, {4})}, 2, output};

    simulation.next();

    EXPECT_FALSE(simulation.finished());
    ASSERT_EQ(simulation.machines_.size(), 1);
    EXPECT_TRUE(simulation.machines_[0].processing());
    EXPECT_EQ(simulation.tick_, 4);
}

TEST(SimulationBlackBoxTests, NextStartsAllInitiallyAvailableMachinesAndJumpsToNearestFinish) {
    std::ostringstream output;
    Simulation simulation{
        {make_machine(0, {{.index = 0, .type = 0}}, {5}), make_machine(1, {{.index = 1, .type = 0}}, {2})}, 2, output};

    simulation.next();

    EXPECT_EQ(simulation.tick_, 2);
    ASSERT_EQ(simulation.machines_.size(), 2);
    EXPECT_EQ(simulation.machines_[0].current_processing(), (product_t{.index = 0, .type = 0}));
    EXPECT_EQ(simulation.machines_[0].current_processing_time(), 5);
    EXPECT_EQ(simulation.machines_[1].current_processing(), (product_t{.index = 1, .type = 0}));
    EXPECT_EQ(simulation.machines_[1].current_processing_time(), 2);
    EXPECT_EQ(simulation.finish_events_.size(), 2);
    EXPECT_EQ(simulation.machine_versions_, (std::vector<uint64_t>{1, 1}));
}

TEST(SimulationBlackBoxTests, NextFinishesCurrentItemAndStartsNextQueuedItem) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}, {.index = 1, .type = 0}}, {3})}, 2, output};

    simulation.next();
    simulation.next();

    EXPECT_EQ(simulation.tick_, 6);
    ASSERT_EQ(simulation.machines_.size(), 1);
    EXPECT_TRUE(simulation.machines_[0].processing());
    EXPECT_FALSE(simulation.machines_[0].ready());
    EXPECT_EQ(simulation.machines_[0].queue_size(), 0);
    EXPECT_EQ(simulation.machines_[0].current_processing(), (product_t{.index = 1, .type = 0}));
    EXPECT_EQ(*simulation.wait_index_.begin(), (std::pair<simtime_t, machine_t>{0, 0}));
}

TEST(SimulationBlackBoxTests, NextRoutesNonFinalProductToSelectedIdleMachine) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}}, {1, 4}), make_machine(1, {}, {1, 4})}, 3, output};

    simulation.next();
    simulation.next();

    EXPECT_EQ(simulation.tick_, 5);
    ASSERT_EQ(simulation.machines_.size(), 2);
    EXPECT_TRUE(simulation.machines_[0].processing());
    EXPECT_EQ(simulation.machines_[0].current_processing(), (product_t{.index = 0, .type = 1}));
    EXPECT_TRUE(simulation.machines_[1].idle());
    EXPECT_EQ(simulation.machines_[1].queue_size(), 0);
}

TEST(SimulationBlackBoxTests, NextQueuesRoutedProductWhenSelectedMachineIsBusy) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}, {.index = 1, .type = 1}}, {1, 5}),
                           make_machine(1, {{.index = 2, .type = 1}}, {1, 10})},
                          3,
                          output};

    simulation.next();
    simulation.next();

    EXPECT_EQ(simulation.tick_, 6);
    ASSERT_EQ(simulation.machines_.size(), 2);
    EXPECT_EQ(simulation.machines_[0].current_processing(), (product_t{.index = 1, .type = 1}));
    EXPECT_EQ(simulation.machines_[1].current_processing(), (product_t{.index = 2, .type = 1}));
    EXPECT_EQ(simulation.machines_[1].queue_size(), 1);
    EXPECT_EQ(simulation.machines_[1].next_item(), (product_t{.index = 0, .type = 1}));
    EXPECT_EQ(*simulation.wait_index_.begin(), (std::pair<simtime_t, machine_t>{0, 0}));
}

TEST(SimulationBlackBoxTests, NextProcessesInstantCascadeBeforeReturning) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}}, {0, 0})}, 3, output};

    simulation.next();

    EXPECT_EQ(simulation.tick_, 0);
    EXPECT_TRUE(simulation.finished());
    ASSERT_EQ(simulation.machines_.size(), 1);
    EXPECT_TRUE(simulation.machines_[0].idle());
    EXPECT_FALSE(simulation.machines_[0].ready());
    EXPECT_FALSE(simulation.machines_[0].has_next());
}

TEST(SimulationBlackBoxTests, NextProcessesMultipleInstantQueuedProductsAtSameTime) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}, {.index = 1, .type = 0}}, {0})}, 2, output};

    simulation.next();

    EXPECT_EQ(simulation.tick_, 0);
    EXPECT_TRUE(simulation.finished());
    ASSERT_EQ(simulation.machines_.size(), 1);
    EXPECT_EQ(simulation.machines_[0].queue_size(), 0);
    EXPECT_FALSE(simulation.machines_[0].processing());
    EXPECT_FALSE(simulation.machines_[0].ready());
}

TEST(SimulationBlackBoxTests, RunConsumesAllProductsAndStopsAtLastFinishTime) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {{.index = 0, .type = 0}, {.index = 1, .type = 0}}, {3})}, 2, output};

    simulation.run();

    EXPECT_EQ(simulation.tick_, 6);
    EXPECT_TRUE(simulation.finished());
    ASSERT_EQ(simulation.machines_.size(), 1);
    EXPECT_TRUE(simulation.machines_[0].idle());
    EXPECT_FALSE(simulation.machines_[0].has_next());
    EXPECT_FALSE(simulation.machines_[0].ready());
}

TEST(SimulationBlackBoxTests, RunLeavesEmptyWorkshopAtZero) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {}, {1}), make_machine(1, {}, {5})}, 2, output};

    simulation.run();

    EXPECT_EQ(simulation.tick_, 0);
    EXPECT_TRUE(simulation.finished());
}

TEST(SimulationBlackBoxTests, AdvanceToNextEventChoosesNearestProcessingCompletion) {
    std::ostringstream output;
    Simulation simulation{
        {make_machine(0, {{.index = 0, .type = 0}}, {5}), make_machine(1, {{.index = 1, .type = 0}}, {2})}, 2, output};
    ASSERT_FALSE(simulation.machines_[0].start());
    ASSERT_FALSE(simulation.machines_[1].start());

    simulation.advance_to_next_event();

    EXPECT_EQ(simulation.tick_, 2);
}

TEST(SimulationBlackBoxTests, AdvanceToNextEventKeepsTickWhenNothingIsProcessing) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {}, {5})}, 2, output};
    simulation.tick_ = 42;

    simulation.advance_to_next_event();

    EXPECT_EQ(simulation.tick_, 42);
}

TEST(SimulationBlackBoxTests, LogMethodsAreCallableForValidValues) {
    std::ostringstream output;
    Simulation simulation{{make_machine(0, {}, {1})}, 2, output};

    EXPECT_NO_THROW(simulation.log_start(1, 2, 0, 0));
    EXPECT_NO_THROW(simulation.log_finish(2, 2, 0, 0));
    EXPECT_NO_THROW(simulation.log_wait(2, 2, 1, 0, 3));
    EXPECT_NO_THROW(simulation.log_ready(3, 2, 0));
    EXPECT_NO_THROW(simulation.log_stop(4));
}

}  // namespace
}  // namespace sim
