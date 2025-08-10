// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include <stdexcept>


class WndHandle {
public:
    WndHandle() = default;

    WndHandle(HWND hWnd) : hWnd(hWnd) {}

    ~WndHandle() {
        clean();
    }

    WndHandle(const WndHandle&) = delete;
    WndHandle& operator=(const WndHandle&) = delete;

    WndHandle(WndHandle&& other) noexcept
        : hWnd(other.hWnd) {
        other.drop();
    }

    WndHandle& operator=(WndHandle&& other) noexcept {
        if (this != &other) {
            clean();
            hWnd = other.hWnd;
            other.drop();
        }
        return *this;
    }

    operator bool() const {
        return hWnd != nullptr;
    }

    HWND get() const {
        return hWnd;
    }

private:
    HWND hWnd = nullptr;

    void clean() {
        if (hWnd != nullptr) {
            DestroyWindow(hWnd);
        }
        drop();
    }

    void drop() {
        hWnd = nullptr;
    }
};


class DCHandle {
public:
    DCHandle() = default;
    
    DCHandle(HDC hDC) : hDC(hDC) {}

    ~DCHandle() {
        clean();
    }

    DCHandle(const DCHandle&) = delete;
    DCHandle& operator=(const DCHandle&) = delete;

    DCHandle(DCHandle&& other) noexcept
        : hDC(other.hDC) {
        other.drop();
    }

    DCHandle& operator=(DCHandle&& other) noexcept {
        if (this != &other) {
            clean();
            hDC = other.hDC;
            other.drop();
        }
        return *this;
    }

    operator bool() const {
        return hDC != nullptr;
    }

    HDC get() const {
        return hDC;
    }

private:
    HDC hDC = nullptr;

    void clean() {
        if (hDC != nullptr) {
            ReleaseDC(WindowFromDC(hDC), hDC);
        }
        drop();
    }
    
    void drop() {
        hDC = nullptr;
    }
};


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




class GLManager {
public:
    GLManager(WndHandle& wnd)
        : dc(makeDC(wnd)),
        gl(std::make_shared<GLHandle>(createModernGLContext(dc.get()))) {}

    GLManager(WndHandle& wnd, const GLManager& other)
        : dc(makeDC(wnd)), gl(other.gl) {}

    GLManager(const GLManager&) = delete;
    GLManager& operator=(const GLManager&) = delete;
    GLManager(const GLManager&&) = delete;
    GLManager& operator=(const GLManager&&) = delete;

    void activate() {
        wglMakeCurrent(dc.get(), gl->get());
    }

    void swapBuffers() {
        SwapBuffers(dc.get());
    }

private:
    DCHandle dc;
    std::shared_ptr<GLHandle> gl;

    static DCHandle makeDC(WndHandle& wnd) {
        DCHandle dc(GetWindowDC(wnd.get()));
        if (!dc) {
            throw std::runtime_error("Cannot get window DC");
        }
        setPixelFormat(dc);
        return dc;
    }

    static void setPixelFormat(DCHandle& dc) {
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags =
            PFD_DRAW_TO_WINDOW |
            PFD_SUPPORT_OPENGL |
            PFD_DOUBLEBUFFER |
            PFD_SUPPORT_COMPOSITION;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int pfi = ChoosePixelFormat(dc.get(), &pfd);
        if (!pfi) {
            throw std::runtime_error("Failed to choose pixel format");
        }
        if (!SetPixelFormat(dc.get(), pfi, &pfd)) {
            throw std::runtime_error("Failed to set pixel format");
        }
    }
};
