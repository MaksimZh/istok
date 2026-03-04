// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>


namespace Istok::EventHandlers {

struct Close {
    std::move_only_function<void() noexcept> func;
};

} // namespace Istok::EventHandlers
