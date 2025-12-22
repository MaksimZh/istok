// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "window.hpp"

#include <gui/gl/base.hpp>

#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include <stdexcept>

namespace Istok::GUI::WinAPI {


class GLHandle {
public:
    GLHandle() : hGL(nullptr) {}
    GLHandle(HGLRC hGL) : hGL(hGL) {}

    GLHandle(const GLHandle&) = delete;
    GLHandle& operator=(const GLHandle&) = delete;
    
    GLHandle(GLHandle&& other) {
        hGL = other.hGL;
        other.drop();
    }

    GLHandle& operator=(GLHandle&& other) {
        if (this != &other) {
            clean();
            hGL = other.hGL;
            other.drop();
        }
        return *this;
    }

    ~GLHandle() {
        clean();
    }

    operator bool() const noexcept {
        return hGL != nullptr;
    }

    void makeCurrent(const DCHandle& dc) {
        if (!*this) {
            throw std::runtime_error("Operating empty GLHandle");
        }
        if (!wglMakeCurrent(dc.getDC(), hGL)) {
            throw std::runtime_error("Failed to make OpenGL context current!");
        }
    }

    void release() {
        if (!*this) {
            throw std::runtime_error("Operating empty GLHandle");
        }
        if (wglGetCurrentContext() == hGL) {
            wglMakeCurrent(nullptr, nullptr);
        }
    }

private:
    HGLRC hGL;

    void drop() {
        hGL = nullptr;
    }

    void clean() {
        if (hGL) {
            release();
            wglDeleteContext(hGL);
        }
    }
};


class CompatibilityGLContext {
public:
    CompatibilityGLContext(const DCHandle& dc)
    : gl(wglCreateContext(dc.getDC())) {
        if (!gl) {
            throw std::runtime_error("Failed to create compatibility OpenGL context");
        }
    }

    CompatibilityGLContext(const CompatibilityGLContext&) = delete;
    CompatibilityGLContext& operator=(const CompatibilityGLContext&) = delete;
    CompatibilityGLContext(CompatibilityGLContext&& other) = delete;
    CompatibilityGLContext& operator=(CompatibilityGLContext&& other) = delete;

    void makeCurrent(const DCHandle& dc) {
        gl.makeCurrent(dc);
    }

private:
    GLHandle gl;
};


class GLContext {
public:
    GLContext() = default;
    GLContext(HWND hWnd) : gl(makeGL(hWnd)) {}

    GLContext(const GLContext&) = delete;
    GLContext& operator=(const GLContext&) = delete;
    
    GLContext(GLContext&& other) = default;
    GLContext& operator=(GLContext&& other) = default;

    operator bool() const noexcept {
        return gl;
    }

    void makeCurrent(const DCHandle& dc) {
        gl.makeCurrent(dc);
    }

    void release() {
        gl.release();
    }

private:
    GLHandle gl;

    static HGLRC makeGL(HWND hWnd) {
        DCHandle dc(hWnd);
        initGLEW(dc);
        
        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_FLAGS_ARB, 0,
            0
        };
        HGLRC hGL = wglCreateContextAttribsARB(dc.getDC(), NULL, attribs);
        if (!hGL) {
            throw std::runtime_error("Failed to create OpenGL context");
        }
        return hGL;
    }

    static void initGLEW(const DCHandle& dc) {
        static bool initialized = false;
        if (initialized) {
            return;
        }
        
        CompatibilityGLContext gl(dc);
        gl.makeCurrent(dc);
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("glewInit failed");
        }
        if (wglewIsSupported("WGL_ARB_create_context") != GL_TRUE) {
            throw std::runtime_error("Modern OpenGL not supported");
        }
    }
};


class WGL {
public:
    class Scope {
    public:
        Scope(GLContext& gl, HWND hWnd)
        : gl(gl), dc(hWnd) {
            gl.makeCurrent(dc);
        }

        ~Scope() {
            gl.release();
        }

        void swapBuffers() {
            SwapBuffers(dc.getDC());
        }

    private:
        GLContext& gl;
        DCHandle dc;
    };
    
    class Owner {
    public:
        Owner() = default;
        
        Owner(Scope& context) : handle(wglGetCurrentContext()) {}

        bool isCurrent() const noexcept {
            return wglGetCurrentContext() == handle;
        }
    private:
        HGLRC handle;
    };
};


} // namespace Istok::GUI::WinAPI
