// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <GL/glew.h>

#include <concepts>
#include <stdexcept>

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


template <OpenGLContext GL, typename Deleter>
class ResourceHandle {
public:
    ResourceHandle() = default;
    
    ResourceHandle(GL::Scope& scope, GLuint handle)
    : owner(scope), handle(handle) {}

    operator bool() const noexcept {
        return handle != 0;
    }

    GLuint get(GL::Scope& scope) const {
        ensureContext();
        return handle;
    }

    ~ResourceHandle() {
        safeDestroy();
    }

    void destroy(GL::Scope& scope) {
        ensureContext();
        safeDestroy();
    }

    ResourceHandle(const ResourceHandle&) = delete;
    ResourceHandle& operator=(const ResourceHandle&) = delete;

    ResourceHandle(ResourceHandle&& other)
    : handle(other.handle), owner(std::move(other.owner)) {
        other.drop();
    }

    ResourceHandle& operator=(ResourceHandle&& other) {
        if (&other == this) {
            return *this;
        }
        if (*this) {
            ensureContext();
            safeDestroy();
        }
        handle = other.handle;
        owner = std::move(other.owner);
        other.drop();
        return *this;
    }

private:
    GL::Owner owner;
    GLuint handle = 0;

    void safeDestroy() noexcept {
        if (*this && owner.isCurrent()) {
            Deleter::destroy(handle);
            handle = 0;
        }
    }

    void ensureContext() const {
        if (!owner.isCurrent()) {
            throw std::runtime_error("Out of owning scope");
        }
    }

    void drop() {
        handle = 0;
    }
};

} // namespace Istok::GUI::OpenGL
