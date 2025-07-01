#include <gui/core/tools.hpp>
#include <gui/core/widget.hpp>
#include <gui/core/dispatcher.hpp>
#include <gui/winapi/window.hpp>

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <map>
#include <set>
#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GL/wglew.h>

/*
class WinAPIMonitorManager {
public:
    WinAPIMonitorManager() {
        update();
    }
    
    bool hasChanged() const {
        return changed;
    }

    void markRead() {
        changed = false;
    }

    const std::vector<MonitorInfo>& getMonitors() const {
        return monitors;
    }

private:
    std::vector<MonitorInfo> monitors;
    bool changed = true;

    void update() {
        EnumDisplayMonitors(
            NULL, NULL, MonitorEnumProc,
            reinterpret_cast<LPARAM>(&monitors));
    }
    
    static BOOL CALLBACK MonitorEnumProc(
        HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData
    ) {
        MONITORINFOEX info = {};
        info.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(hMonitor, &info);
        auto dest = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
        dest->push_back(MonitorInfo{
            toUTF8(info.szDevice),
            {
                info.rcMonitor.left, info.rcMonitor.top,
                info.rcMonitor.right, info.rcMonitor.bottom,
            },
            {
                info.rcWork.left, info.rcWork.top,
                info.rcWork.right, info.rcWork.bottom,
            }});
        return TRUE;
    }
};
*/

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


class Window: public Widget {};


class WindowManager {
public:
    void update(Window& window, Position<float> pos) {
        if (!wsMap.contains(&window)) {
            Size<float> size = window.getSize();
            Position<int> sysPos{
                static_cast<int>(std::round(pos.x)),
                static_cast<int>(std::round(pos.y))
            };
            Size<int> sysSize{
                static_cast<int>(std::round(size.width)),
                static_cast<int>(std::round(size.height))
            };
            wsMap[&window] = std::make_unique<SysWindow>("Istok", sysPos, sysSize);
            wsMap[&window]->show();
            swMap[wsMap[&window].get()] = &window;
        }
    }

    SysWindow& getSysWindow(Window& window) {
        return *wsMap[&window];
    }

private:
    std::map<Window*, std::unique_ptr<SysWindow>> wsMap;
    std::map<SysWindow*, Window*> swMap;
};


class MyScreen: RootWidget<Window> {
public:
    void addWindow(Window& window, Position<float> pos) {
        attach(window);
        windows.insert(&window);
        windowManager.update(window, pos);
        if (!gl) {
            gl = std::make_unique<ModernGLContext>(windowManager.getSysWindow(window).getDC());
        }
    }

    void clear() {
        for (auto& window : windows) {
            SysWindow& sysWindow = windowManager.getSysWindow(*window);
            wglMakeCurrent(sysWindow.getDC(), gl->getGL());
            glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            SwapBuffers(sysWindow.getDC());
        }
    }

private:
    WindowManager windowManager;
    std::set<Window*> windows;
    std::unique_ptr<ModernGLContext> gl;
};



int main() {
    MyScreen screen;
    Window window1;
    Window window2;
    window1.setSize({400, 300});
    window2.setSize({500, 200});
    screen.addWindow(window1, {300, 200});
    screen.addWindow(window2, {400, 100});

    while (true) {
        MSG msg;
        GetMessageW(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        screen.clear();
    }

    return 0;
}
