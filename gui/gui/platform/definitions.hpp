// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <variant>
#include <type_traits>
#include <optional>
#include <string>
#include <memory>

namespace Istok::GUI {

namespace Event {
    /// @brief Response to a heartbeat request, confirming platform is alive
    /// @note Sent automatically by the platform thread upon receiving HeartbeatRequest
    struct PlatformHeartbeatResponse {};
    
    /// @brief Heartbeat response was not received within the expected time
    /// @note Indicates the platform may have crashed or become unresponsive
    struct PlatformHeartbeatTimeout {};

    /// @brief Exception encountered in the platform
    struct PlatformException {
        std::exception_ptr exception; ///< Exception stored for later handling
    };

    /// @brief Platform has shut down
    struct PlatformShutdown {};
    
    /// @brief Request to close a window
    /// @tparam ID Window identifier type
    template <typename ID>
    struct WindowClose { ID id; };
}

template <typename ID>
using PlatformEvent = std::variant<
    Event::PlatformHeartbeatResponse,
    Event::PlatformHeartbeatTimeout,
    Event::PlatformException,
    Event::PlatformShutdown,
    Event::WindowClose
>;


template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};

struct WindowParams {
    Rect<int> location;
    std::optional<std::string> title;
};


template <typename ID>
struct Scene {};


template <typename ID>
struct ScenePatch {};


template <typename Platform>
concept GUIPlatform = requires {
    typename Platform::ID;
    requires std::is_default_constructible_v<Platform>;
} && requires(
    Platform platform,
    typename Platform::ID id,
    WindowParams windowParams,
    Rect<int> location,
    std::unique_ptr<Scene<typename Platform::ID>>&& scene,
    std::unique_ptr<ScenePatch<typename Platform::ID>>&& scenePatch
) {
    {platform.getMessage()} noexcept ->
        std::same_as<PlatformEvent<typename Platform::ID>>;
    {platform.createWindow(id, windowParams)} -> std::same_as<void>;
    {platform.destroyWindow(id)} -> std::same_as<void>;
    {platform.setWindowLocation(id, location)} -> std::same_as<void>;
    {platform.loadScene(id, std::move(scene))} -> std::same_as<void>;
    {platform.patchScene(id, std::move(scenePatch))} -> std::same_as<void>;
};

} // namespace Istok::GUI
