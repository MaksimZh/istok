// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <GL/glew.h>
#include <GL/wglew.h>

#include <stdexcept>


class GLHandle {
public:
    GLHandle(HGLRC hGL) {
        this->hGL = hGL;
    }

    ~GLHandle() {
        clean();
    }

    GLHandle(const GLHandle&) = delete;
    GLHandle& operator=(const GLHandle&) = delete;
    
    GLHandle(GLHandle&& other) noexcept
        : hGL(other.hGL)
    {
        other.drop();
    }
    
    GLHandle& operator=(GLHandle&& other) noexcept {
        if (this != &other) {
            clean();
            hGL = other.hGL;
            other.drop();
        }
        return *this;
    }

    operator bool() const {
        return hGL != nullptr;
    }

    HGLRC get() const { return hGL; }

private:
    HGLRC hGL = nullptr;

    void clean() {
        if (hGL == nullptr) {
            return;
        }
        if (wglGetCurrentContext() == hGL) {
            wglMakeCurrent(nullptr, nullptr);
        }
        wglDeleteContext(hGL);
        drop();
    }

    void drop() {
        hGL = nullptr;
    }
};


GLHandle createCompatibilityGLContext(HDC hDC) {
    GLHandle gl(wglCreateContext(hDC));
    if (!gl) {
        throw std::runtime_error("Failed to create OpenGL context");
    }
    return gl;
}


void initGLEW(HDC hDC) {
    static bool initialized = false;
    if (initialized) {
        return;
    }
    GLHandle tmp = createCompatibilityGLContext(hDC);
    if (!wglMakeCurrent(hDC, tmp.get())) {
        throw std::runtime_error("Failed to make OpenGL context current!");
    }
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("glewInit failed");
    }
    if (wglewIsSupported("WGL_ARB_create_context") != GL_TRUE) {
        throw std::runtime_error("Modern OpenGL not supported");
    }
}


GLHandle createModernGLContext(HDC hDC) {
    initGLEW(hDC);

    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, 0,
        0
    };
    GLHandle gl(wglCreateContextAttribsARB(hDC, NULL, attribs));
    if (!gl) {
        throw std::runtime_error("Failed to create OpenGL context");
    }
    return gl;
}
