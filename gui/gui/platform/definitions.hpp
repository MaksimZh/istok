// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <variant>
#include <type_traits>
#include <optional>
#include <string>
#include <memory>

namespace Istok::GUI {

namespace PlatformCommands {
    /// @brief Heartbeat request, checking if platform is alive
    struct HeartbeatRequest {};
}

namespace PlatformEvents {
    /// @brief Response to a heartbeat request, confirming platform is alive
    struct HeartbeatResponse {};

    /// @brief Exception encountered in the platform
    struct Exception {
        std::exception_ptr exception; ///< Exception stored for later handling
    };

    /// @brief Platform has shut down
    struct Shutdown {};
    
    /// @brief Request to close a window
    /// @tparam ID Window identifier type
    template <typename ID>
    struct WindowClose { ID id; };
}

template <typename ID>
using PlatformEvent = std::variant<
    PlatformEvents::Exception,
    PlatformEvents::Shutdown,
    PlatformEvents::WindowClose<ID>
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


template <typename Platform>
concept GUIPlatform = requires {
    typename Platform::ID;
    typename Platform::Renderer;
    requires std::is_default_constructible_v<Platform>;
} && requires(
    Platform platform,
    typename Platform::ID id,
    WindowParams windowParams,
    std::unique_ptr<typename Platform::Renderer>&& renderer,
    std::unique_ptr<typename Platform::Renderer::Scene>&& scene
) {
    {platform.getMessage()} noexcept ->
        std::same_as<PlatformEvent<typename Platform::ID>>;
    {platform.createWindow(id, windowParams)} noexcept -> std::same_as<void>;
    {platform.destroyWindow(id)} noexcept -> std::same_as<void>;
    {platform.setRenderer(id, std::move(renderer))} noexcept ->
        std::same_as<void>;
    {platform.loadScene(id, std::move(scene))} noexcept -> std::same_as<void>;
};

} // namespace Istok::GUI
