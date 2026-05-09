#include "core/resources.hpp"

size_t std::hash<tvb::core::Resource>::operator()(const tvb::core::Resource &r) const noexcept {
    auto type_hash = std::hash<std::underlying_type_t<tvb::core::ResourceType>>{}(
        static_cast<std::underlying_type_t<tvb::core::ResourceType>>(r.type_));

    auto value_hash = std::hash<std::uint16_t>{}(r.value_);

    return type_hash ^ (value_hash << 1);
}
