// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <type_traits>

namespace Istok::GUI::OpenGL {

template <typename Owner>
concept OpenGLContextOwner = std::movable<Owner>
&& requires(Owner owner) {
    {owner.isCurrent()} noexcept -> std::same_as<bool>;
};

template <typename GL>
concept OpenGLContext = requires {
    typename GL::Scope;
    typename GL::Owner;
} && OpenGLContextOwner<typename GL::Owner>;

} // namespace Istok::GUI::OpenGL
