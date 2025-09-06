// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "definitions.hpp"
#include <tools/queue.hpp>
#include <tools/helpers.hpp>

#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include <memory>
#include <unordered_map>

namespace Istok::GUI::WinAPI {


HINSTANCE getHInstance() {
    static HINSTANCE hInstance =
        reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
    return hInstance;
}


class WindowClass {
public:
    WindowClass() = default;

    WindowClass(WNDPROC lpfnWndProc, LPCWSTR className)
        : name(className)
    {
        WNDCLASSEX wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_OWNDC;
        wcex.lpfnWndProc = lpfnWndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = getHInstance();
        wcex.hIcon = nullptr;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = className;
        wcex.hIconSm = nullptr;
        if (!RegisterClassEx(&wcex)) {
            throw std::runtime_error("Failed to register window class.");
        }
    }

    ~WindowClass() noexcept {
        UnregisterClass(name, getHInstance());
    }

    WindowClass(const WindowClass&) = delete;
    WindowClass& operator=(const WindowClass&) = delete;
    WindowClass(WindowClass&& other) = delete;
    WindowClass& operator=(WindowClass&& other) = delete;

    LPCWSTR get() const {
        return name;
    }

private:
    LPCWSTR name = nullptr;
};


class MessageHandler {
public:
    virtual void onClose() noexcept = 0;
    virtual void onPaint() noexcept = 0;
};


class DCHandle {
public:
    DCHandle(HDC hDC) : hDC(hDC) {}
    
    DCHandle(HWND hWnd) : hDC(GetDC(hWnd)) {
        if (hDC == nullptr) {
            throw std::runtime_error("Failed to get window DC");
        }
    }

    DCHandle(const DCHandle&) = delete;
    DCHandle& operator=(const DCHandle&) = delete;
    
    DCHandle(DCHandle&& other) {
        hDC = other.hDC;
        other.drop();
    }

    DCHandle& operator=(DCHandle&& other) {
        if (this != &other) {
            clean();
            hDC = other.hDC;
            other.drop();
        }
        return *this;
    }

    ~DCHandle() {
        clean();
    }

    operator bool() const noexcept {
        return hDC != nullptr;
    }

    HDC get() const noexcept {
        return hDC;
    }

private:
    HDC hDC;

    void drop() {
        hDC = nullptr;
    }

    void clean() {
        if (hDC) {
            ReleaseDC(WindowFromDC(hDC), hDC);
        }
    }
};


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
        if (!wglMakeCurrent(dc.get(), hGL)) {
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
    CompatibilityGLContext(const DCHandle& dc) : gl(wglCreateContext(dc.get())) {
        if (!gl) {
            throw std::runtime_error("Failed to create compatibility OpenGL context");
        }
    }

    CompatibilityGLContext(const CompatibilityGLContext&) = delete;
    CompatibilityGLContext& operator=(const CompatibilityGLContext&) = delete;
    CompatibilityGLContext(CompatibilityGLContext&& other) = delete;
    CompatibilityGLContext& operator=(CompatibilityGLContext&& other) = delete;

    void makeCurrent(const DCHandle& dc) {
        gl.makeCurrent(dc.get());
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
        HGLRC hGL = wglCreateContextAttribsARB(dc.get(), NULL, attribs);
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


class HWndWindow {
public:
    HWndWindow(WindowParams params, MessageHandler& handler)
        : handler(handler)
    {
        hWnd = CreateWindowEx(
            params.title.has_value() ? NULL : WS_EX_TOOLWINDOW,
            getWindowClass(),
            toUTF16(params.title.value_or("")).c_str(),
            WS_POPUP,
            params.location.left, params.location.top,
            params.location.right - params.location.left,
            params.location.bottom - params.location.top,
            NULL, NULL, getHInstance(), nullptr);
        if (!hWnd) {
            throw std::runtime_error("Cannot create window");
        }
        SetWindowLongPtr(
            hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        ShowWindow(hWnd, SW_SHOW);
    }

    ~HWndWindow() noexcept {
        if (hWnd) {
            DestroyWindow(hWnd);
        }
    }

    struct SysContext {
        HWND hWnd;
    };

    SysContext sysContext() const noexcept {
        return SysContext(hWnd);
    }

private:
    MessageHandler& handler;
    HWND hWnd = nullptr;

    static LPCWSTR getWindowClass() {
        static WindowClass wc(windowProc, L"Istok");
        return wc.get();
    }

    static LRESULT CALLBACK windowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (auto handler = reinterpret_cast<HWndWindow*>(
                GetWindowLongPtr(hWnd, GWLP_USERDATA)))
        {
            return handler->handleMessage(hWnd, msg, wParam, lParam);
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    LRESULT handleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CLOSE:
            handler.onClose();
            return 0;
        case WM_PAINT: {
            handler.onPaint();
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;
        }
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    static std::wstring toUTF16(const std::string& source) {
        int size = MultiByteToWideChar(
            CP_UTF8, 0, source.c_str(), -1, nullptr, 0);
        if (size == 0) {
            throw std::runtime_error("UTF-8 to UTF-16 conversion failed");
        }
        std::wstring result(size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, &result[0], size);
        return result;
    }
};


void prepareForGL(HWndWindow& window) {
    DCHandle dc(window.sysContext().hWnd);
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


class CurrentGL {
public:
    CurrentGL(GLContext& gl, HWndWindow& window) : gl(gl), dc(window.sysContext().hWnd) {
        gl.makeCurrent(dc);
    }

    ~CurrentGL() {
        SwapBuffers(dc.get());
        gl.release();
    }

private:
    GLContext& gl;
    DCHandle dc;
};


template <typename SysWindow, typename Renderer>
class WindowCore {
public:
    WindowCore(WindowParams params, MessageHandler& handler)
        : window(params, handler) {}

    void setRenderer(std::unique_ptr<Renderer>&& renderer) {
        this->renderer = std::move(renderer);
    }

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        getRenderer().loadScene(std::move(scene));
    }

    void draw() {
        getRenderer().draw(window);
    }

private:
    SysWindow window;
    std::unique_ptr<Renderer> renderer;

    Renderer& getRenderer() {
        if (renderer == nullptr) {
            throw std::runtime_error("Renderer not attached");
        }
        return *renderer;
    }
};


template <typename Window>
class EventHandler {
public:
    virtual void onException(std::exception_ptr exception) noexcept = 0;
    virtual void onClose(Window* sender) noexcept = 0;
};


template <typename SysWindow, typename Renderer>
class Window: public MessageHandler {
public:
    Window(WindowParams params, EventHandler<Window>& handler)
        : core(params, *this), handler(handler) {}

    void onClose() noexcept override {
        handler.onClose(this);
    }

    void onPaint() noexcept override {
        try {
            core.draw();
        } catch(...) {
            handler.onException(std::current_exception());
        }
    }

    void setRenderer(std::unique_ptr<Renderer>&& renderer) {
        core.setRenderer(std::move(renderer));
    }

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        core.loadScene(std::move(scene));
    }

private:
    WindowCore<SysWindow, Renderer> core;
    EventHandler<Window>& handler;
};


template <typename ID, typename Window>
class WindowMap {
public:
    void insert(ID id, std::unique_ptr<Window> window) {
        if (identifiers.contains(window.get()) || windows.contains(id)) {
            throw std::runtime_error("Window overwrite");
        }
        identifiers[window.get()] = id;
        windows[id] = std::move(window);
    }

    void erase(ID id) {
        if (!windows.contains(id)) {
            throw std::runtime_error("Window not found");
        }
        Window* window = windows[id].get();
        windows.erase(id);
        assert(identifiers.contains(window));
        identifiers.erase(window);
    }

    ID getID(Window* window) {
        if (!identifiers.contains(window)) {
            throw std::runtime_error("Window not found");
        }
        return identifiers[window];
    }

    Window& getWindow(ID id) {
        if (!windows.contains(id)) {
            throw std::runtime_error("Window not found");
        }
        return *windows[id];
    }

private:
    std::unordered_map<ID, std::unique_ptr<Window>, Tools::hash<ID>> windows;
    std::unordered_map<Window*, ID> identifiers;
};


template <typename ID, typename Window>
class WindowManager {
public:
    WindowManager(EventHandler<Window>& handler)
        : handler(handler) {}

    void create(ID id, WindowParams params) {
        windows.insert(id, std::make_unique<Window>(params, handler));
    }

    void destroy(ID id) {
        windows.erase(id);
    }

    ID getID(Window* window) {
        return windows.getID(window);
    }

    Window& getWindow(ID id) {
        return windows.getWindow(id);
    }

private:
    EventHandler<Window>& handler;
    WindowMap<ID, Window> windows;
};


template <typename ID_, typename SysWindow, typename Renderer>
class Platform: public EventHandler<Window<SysWindow, Renderer>> {
public:
    using ID = ID_;
    using Window = Window<SysWindow, Renderer>;

    Platform()
        : windows(*this) {}

    PlatformEvent<ID> getMessage() noexcept {
        while (true) {
            if (!outQueue.empty()) {
                return outQueue.take();
            }
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                return Event::PlatformShutdown{};
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void createWindow(ID id, WindowParams params) {
        windows.create(id, params);
    }

    void destroyWindow(ID id) {
        windows.destroy(id);
    }

    void setRenderer(ID id, std::unique_ptr<Renderer>&& renderer) {
        windows.getWindow(id).setRenderer(std::move(renderer));
    }

    void loadScene(ID id, std::unique_ptr<typename Renderer::Scene>&& scene) {
        windows.getWindow(id).loadScene(std::move(scene));
    }

    void onException(std::exception_ptr exception) noexcept override {
        outQueue.push(Event::PlatformException(exception));
    }

    void onClose(Window* sender) noexcept override {
        try {
            outQueue.push(Event::WindowClose(windows.getID(sender)));
        } catch(...) {
            outQueue.push(Event::PlatformException(std::current_exception()));
        }
    }
    
private:
    WindowManager<ID, Window> windows;
    Tools::SimpleQueue<PlatformEvent<ID>> outQueue;
};

}
