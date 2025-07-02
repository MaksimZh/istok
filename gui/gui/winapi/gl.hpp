// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <GL/glew.h>
#include <GL/wglew.h>

#include <stdexcept>


class GLContext {
public:
    GLContext(HGLRC hGL) {
        this->hGL = hGL;
    }

    ~GLContext() {
        if (!hGL) {
            return;
        }
        if (wglGetCurrentContext() == hGL) {
            wglMakeCurrent(nullptr, nullptr);
        }
        wglDeleteContext(hGL);
    }

    GLContext(const GLContext&) = delete;
    GLContext& operator=(const GLContext&) = delete;
    GLContext(const GLContext&&) = delete;
    GLContext& operator=(const GLContext&&) = delete;

    HGLRC getGL() const { return hGL; }

private:
    HGLRC hGL;
};


class CompatibilityGLContext : public GLContext {
public:
    CompatibilityGLContext(HDC hDC) : GLContext(wglCreateContext(hDC)) {
        if (!getGL()) {
            throw std::runtime_error("Failed to create OpenGL context");
        }
    }

    CompatibilityGLContext(const CompatibilityGLContext&) = delete;
    CompatibilityGLContext& operator=(const CompatibilityGLContext&) = delete;
    CompatibilityGLContext(const CompatibilityGLContext&&) = delete;
    CompatibilityGLContext& operator=(const CompatibilityGLContext&&) = delete;
};


class ModernGLContext : public GLContext {
public:
    ModernGLContext(HDC hDC) : GLContext(createContext(hDC)) {
        if (!getGL()) {
            throw std::runtime_error("Failed to create OpenGL context");
        }
    }

    ModernGLContext(const ModernGLContext&) = delete;
    ModernGLContext& operator=(const ModernGLContext&) = delete;
    ModernGLContext(const ModernGLContext&&) = delete;
    ModernGLContext& operator=(const ModernGLContext&&) = delete;

private:
    static HGLRC createContext(HDC hDC) {
        initGLEW(hDC);

        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_FLAGS_ARB, 0,
            0
        };
        HGLRC hGL = wglCreateContextAttribsARB(hDC, NULL, attribs);
        if (!hGL) {
            throw std::runtime_error("Failed to create OpenGL context");
        }
        return hGL;
    }

    static void initGLEW(HDC hDC) {
        static bool initialized = false;
        if (initialized) {
            return;
        }
        CompatibilityGLContext tmp(hDC);
        if (!wglMakeCurrent(hDC, tmp.getGL())) {
            throw std::runtime_error("Failed to make OpenGL context current!");
        }
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("glewInit failed");
        }
        if (wglewIsSupported("WGL_ARB_create_context") != GL_TRUE) {
            throw std::runtime_error("Modern OpenGL not supported");
        }
    }
};