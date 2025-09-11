// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "base.hpp"

namespace Istok::GUI::OpenGL {

template <OpenGLContext GL>
class VertexBuffer {
public:
    VertexBuffer() = default;

    VertexBuffer(GL::Scope& scope) {
        GLuint value;
        glGenBuffers(1, &value);
        if (!value) {
            throw std::runtime_error("Failed to create OpenGL buffer");
        }
        handle = Handle(scope, value);
    }

    void destroy(GL::Scope& scope) {
        handle.destroy(scope);
    }

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&& other) = default;
    VertexBuffer& operator=(VertexBuffer&& other) = default;

    void bind(GL::Scope& scope) {
        glBindBuffer(GL_ARRAY_BUFFER, handle.get());
    }

private:
    struct Deleter {
        static void destroy(GLuint handle) {
            glDeleteBuffers(1, &handle);
        }
    };

    using Handle = ResourceHandle<GL, Deleter>;
    Handle handle;
};


template <OpenGLContext GL>
class VertexArray {
public:
    VertexArray() = default;

    VertexArray(GL::Scope& scope) {
        GLuint value;
        glGenVertexArrays(1, &value);
        if (!value) {
            throw std::runtime_error("Failed to create OpenGL Array");
        }
        handle = Handle(scope, value);
    }

    void destroy(GL::Scope& scope) {
        handle.destroy(scope);
    }

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&& other) = default;
    VertexArray& operator=(VertexArray&& other) = default;

    void bind(GL::Scope& scope) {
        glBindVertexArray(GL_ARRAY_Array, handle.get());
    }

private:
    struct Deleter {
        static void destroy(GLuint handle) {
            glDeleteVertexArrays(1, &handle);
        }
    };

    using Handle = ResourceHandle<GL, Deleter>;
    Handle handle;
};

} // namespace Istok::GUI::OpenGL
