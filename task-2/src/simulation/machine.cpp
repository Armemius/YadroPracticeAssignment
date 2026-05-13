#include "simulation/machine.hpp"
#include <optional>
#include <queue>
#include <stdexcept>
#include "simulation/types.hpp"

namespace sim {

Machine::Machine(machine_t index, simtime_t &clock, const std::vector<product_t> &products,
                 std::unordered_map<product_type_t, optime_t> optimes)
    : index_(index), clock_(&clock), optimes_(std::move(optimes)) {
    if (products.empty()) {
        return;
    }
    current_processing_till_ = optimes_.at(products.front().type);
    for (const auto &it : products) {
        processing_till_ += optimes_.at(it.type);
    }
    queue_ = std::queue(std::deque<product_t>(products.begin(), products.end()));
}

bool Machine::start() {
    if (current_.has_value()) {
        throw std::logic_error("Machine is already processing item");
    }
    if (result_.has_value()) {
        throw std::logic_error("Slot for output is full, please yield item");
    }
    if (queue_.empty()) {
        throw std::out_of_range("No items in the machine processing queue");
    }
    product_t product = queue_.front();
    queue_.pop();
    optime_t processing_time = optimes_.at(product.type);
    if (processing_time == 0) {
        upgrade_product(product);
        result_ = std::move(product);
        return true;
    }
    current_processing_till_ = *clock_ + processing_time;
    current_ = std::move(product);

    return false;
}

bool Machine::tick() {
    if (!queue_.empty() && !processing()) {
        ++processing_till_;
    }

    processing_till_ = std::max(processing_till_, *clock_);
    current_processing_till_ = std::max(processing_till_, *clock_);

    if (processing_till_ <= *clock_) {
        result_ = std::move(current_);
        current_ = std::nullopt;
    }

    return ready();
}

bool Machine::ready() const noexcept {
    return result_.has_value();
}

bool Machine::processing() const noexcept {
    return current_.has_value();
}

product_t Machine::yield() {
    if (!result_.has_value()) {
        throw std::out_of_range("No ready item for yielding");
    }
    product_t result = *std::move(result_);
    result_ = std::nullopt;
    return result;
}

simtime_t Machine::enqueue(product_t product) {
    processing_till_ += optimes_.at(product.type);
    queue_.push(std::move(product));
    return processing_till_;
}

simtime_t Machine::processing_till() const noexcept {
    return processing_till_;
}

}  // namespace sim