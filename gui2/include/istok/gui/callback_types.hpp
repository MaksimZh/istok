// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>


namespace Istok::GUI {

using QuitCallback = std::move_only_function<void() noexcept>;


namespace EventHandlers {

struct Close {
    std::move_only_function<void() noexcept> func;
};

} // namespace EventHandlers

} // namespace Istok::GUI
