#pragma once

#include <compare>
#include <cstdint>

namespace sim {

/// Index of the product type
using product_type_t = uint16_t;

/// Index of the product
using product_index_t = uint32_t;

/// Struct representing state for certain product
struct Product {
    product_index_t index;
    product_type_t type;

    friend std::strong_ordering operator<=>(const Product &lhs, const Product &rhs) = default;
};

/// Product state
using product_t = Product;

/// Index of the operation type
using operation_t = uint16_t;

/// Index of the machine
using machine_t = uint16_t;

/// Operation time
using optime_t = uint16_t;

/// Time point of simulation
using simtime_t = uint64_t;

/**
     * @brief Upgrades product
     * 
     * @param product Product to upgrade
     */
inline void upgrade_product(product_t &product) {
    ++product.type;
}

}  // namespace sim
