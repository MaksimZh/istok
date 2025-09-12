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
struct Position {
    T x;
    T y;
};

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

enum class WindowArea {
    hole,
    client,
    moving,
    sizingTL,
    sizingT,
    sizingTR,
    sizingR,
    sizingBR,
    sizingB,
    sizingBL,
    sizingL
};

class WindowAreaTester {
public:
    virtual WindowArea testWindowArea(Position<int> position) const noexcept = 0;
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
    Platform::Renderer::Scene&& scene,
    std::unique_ptr<WindowAreaTester>&& areaTester
) {
    {platform.getMessage()} noexcept ->
        std::same_as<PlatformEvent<typename Platform::ID>>;
    {platform.createWindow(id, windowParams)} noexcept -> std::same_as<void>;
    {platform.destroyWindow(id)} noexcept -> std::same_as<void>;
    {platform.setRenderer(id, std::move(renderer))} noexcept ->
        std::same_as<void>;
    {platform.loadScene(id, std::move(scene))} noexcept -> std::same_as<void>;
    {platform.setAreaTester(id, std::move(areaTester))} noexcept ->
        std::same_as<void>;
};

} // namespace Istok::GUI
