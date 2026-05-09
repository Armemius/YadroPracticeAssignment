#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace tvb::core {

/// Represents resource type in the dungeon and their value
enum class ResourceType { IRON, GOLD, GEM, EXPERIENCE };

/// Represents resource
class Resource {
   public:
    friend bool operator==(const Resource &lhs, const Resource &rhs) = default;

    /**
     * @brief Returns resource type
     * 
     * @return constexpr ResourceType type of the resource
     */
    [[nodiscard]] constexpr ResourceType type() const noexcept { return type_; }

    /**
     * @brief Returns resource value
     * 
     * @return constexpr uint16_t value of the resource
     */
    [[nodiscard]] constexpr uint16_t value() const noexcept { return value_; }

   private:
    constexpr Resource(ResourceType type, uint16_t value) : type_(type), value_(value) {}

    ResourceType type_;  ///< Type of resource
    uint16_t value_;     ///< Value of resource

    friend class Resources;
    friend struct std::hash<Resource>;
};

// Struct containing variations of resources
struct Resources final {
    ~Resources() = delete;

    /// Constant resource representing iron
    constexpr static Resource IRON = Resource{ResourceType::IRON, 7};

    /// Constant resource representing gold
    constexpr static Resource GOLD = Resource{ResourceType::GOLD, 11};

    /// Constant resource representing gems
    constexpr static Resource GEM = Resource{ResourceType::GEM, 23};

    /// Constant resource representing experience
    constexpr static Resource EXPERIENCE = Resource{ResourceType::EXPERIENCE, 1};
};

}  // namespace tvb::core

template <>
struct std::hash<tvb::core::Resource> {
    size_t operator()(const tvb::core::Resource &r) const noexcept;
};
