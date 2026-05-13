#include "simulation/machine.hpp"

#include "simulation/types.hpp"

#include <optional>
#include <queue>
#include <stdexcept>

namespace sim {

Machine::Machine(machine_t index, const std::vector<product_t> &products,
                 std::unordered_map<product_type_t, optime_t> optimes)
    : index_(index), optimes_(std::move(optimes)) {
    if (products.empty()) {
        return;
    }
    for (const auto &it : products) {
        queue_time_ += optimes_.at(it.type);
    }
    queue_ = std::queue(std::deque<product_t>(products.begin(), products.end()));
}

machine_t Machine::index() const noexcept {
    return index_;
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
    current_processing_till_ = last_tick_ + processing_time;
    current_ = std::move(product);

    return false;
}

bool Machine::start(product_t product) {
    if (current_.has_value()) {
        throw std::logic_error("Machine is already processing item");
    }
    if (result_.has_value()) {
        throw std::logic_error("Slot for output is full, please yield item");
    }
    if (!queue_.empty()) {
        throw std::out_of_range("Cannot process item when queue is not empty");
    }
    optime_t processing_time = optimes_.at(product.type);
    if (processing_time == 0) {
        upgrade_product(product);
        result_ = std::move(product);
        return true;
    }
    current_processing_till_ = last_tick_ + processing_time;
    current_ = std::move(product);

    return false;
}

bool Machine::tick(simtime_t now) {
    if (last_tick_ >= now) {
        throw std::logic_error("Machine cannot tick backward in time");
    }
    simtime_t diff = now - last_tick_;
    last_tick_ = now;

    if (idle()) {
        queue_time_ += diff;
        return ready();
    }

    if (current_processing_till_ <= now) {
        diff = now - current_processing_till_;
        upgrade_product(*current_);
        result_ = std::move(current_);
        current_ = std::nullopt;
        queue_time_ += diff;
    }

    return ready();
}

simtime_t Machine::last_tick() const noexcept {
    return last_tick_;
}

bool Machine::ready() const noexcept {
    return result_.has_value();
}

bool Machine::idle() const noexcept {
    return !current_.has_value();
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
    queue_time_ += optimes_.at(product.type);
    queue_.push(std::move(product));
    return std::max(queue_time_, last_tick_);
}

simtime_t Machine::current_processing_time() const noexcept {
    return std::max(current_processing_till_, last_tick_);
}

simtime_t Machine::queue_time() const noexcept {
    return std::max(queue_time_, last_tick_);
}

size_t Machine::queue_size() const noexcept {
    return queue_.size();
}

bool Machine::has_next() const noexcept {
    return !queue_.empty();
}

bool Machine::can_process() const noexcept {
    return idle() && has_next();
}

}  // namespace sim
