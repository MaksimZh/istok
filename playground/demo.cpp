#include <ecs.hpp>
#include <gui/winapi/window.hpp>
#include <gui/winapi/gl.hpp>

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <map>
#include <set>
#include <iostream>
#include <cmath>
#include <thread>


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

namespace ecs = Istok::ECS;

struct NeedsSysWindow {};

struct IsToolWindow {};

struct ToShow {};

struct Visible {};

struct SysWindowLink {
    SysWindow* sysWindow;
};

struct Parent {
    ecs::Entity entity;
};

struct Children {
    std::vector<ecs::Entity> entities;
};

struct LocalPosition {
    int left;
    int top;
};

struct RenderingPosition {
    int left;
    int top;
};

struct ScreenPosition {
    int left;
    int top;
};

struct Color {
    float r;
    float g;
    float b;
};


void threadProc(
        MessageQueue<bool>& inQueue,
        GUIMessageQueue<int>& outQueue) {
    int counter = 0;
    while (inQueue.take()) {
        std::cout << "bump" << std::endl;
        outQueue.push(counter++);
    }
    std::cout << "thread end" << std::endl;
}

MessageQueue<bool> messageQueue;
GUIMessageQueue<int> guiQueue;

class SysWindowSystem {
public:
    SysWindowSystem() = default;
    SysWindowSystem(const SysWindowSystem&) = delete;
    SysWindowSystem& operator=(const SysWindowSystem&) = delete;
    SysWindowSystem(SysWindowSystem&&) = default;
    SysWindowSystem& operator=(SysWindowSystem&&) = default;

    void run(ecs::EntityComponentManager& manager) {
        for (auto e : manager.view<NeedsSysWindow>()) {
            Size<int> size = manager.has<Size<int>>(e)
                ? manager.get<Size<int>>(e)
                : Size<int>(64, 64);
            ScreenPosition pos = manager.has<ScreenPosition>(e)
                ? manager.get<ScreenPosition>(e)
                : ScreenPosition(0, 0);
            sysWindows.push_back(
                std::make_unique<SysWindow>(
                    "Istok",
                    Position<int>(pos.left, pos.top),
                    size,
                    manager.has<IsToolWindow>(e),
                    messageQueue, guiQueue
                ));
            SysWindow* sw = sysWindows.back().get();
            manager.remove<NeedsSysWindow>(e);
            manager.set(e, SysWindowLink{sw});
        }

        for (auto e : manager.view<SysWindowLink, ToShow>()) {
            SysWindow* sysWindow = manager.get<SysWindowLink>(e).sysWindow;
            sysWindow->show();
            manager.remove<ToShow>(e);
            manager.set(e, Visible{});
        }

        if (!gl && sysWindows.size() > 0) {
            gl = std::make_unique<ModernGLContext>(sysWindows.front()->getDC());
        }

        for (auto e : manager.view<SysWindowLink, Visible, Color>()) {
            SysWindow* sw = manager.get<SysWindowLink>(e).sysWindow;
            wglMakeCurrent(sw->getDC(), gl->getGL());
            Color color = manager.get<Color>(e);
            glClearColor(color.r, color.g, color.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            SwapBuffers(sw->getDC());
        }
    }

private:
    std::vector<std::unique_ptr<SysWindow>> sysWindows;
    std::unique_ptr<ModernGLContext> gl;
};


void locateWindows(ecs::EntityComponentManager& manager, ecs::Entity root) {
    if (manager.has<SysWindowLink>(root)) {
        SysWindow* sw = manager.get<SysWindowLink>(root).sysWindow;
        RECT rect;
        GetWindowRect(sw->getHWND(), &rect);
        manager.set(root, ScreenPosition(rect.left, rect.top));
    }
    if (!manager.has<Children>(root) || !manager.has<ScreenPosition>(root)) {
        return;
    }
    ScreenPosition screen = manager.get<ScreenPosition>(root);
    for (auto e : manager.get<Children>(root).entities) {
        LocalPosition local = manager.get<LocalPosition>(e);
        manager.set(e, ScreenPosition(
            screen.left + local.left,
            screen.top + local.top));
        locateWindows(manager, e);
    }
}


int main() {
    ecs::EntityComponentManager manager;
    ecs::Entity window = manager.createEntity();
    manager.set(window, NeedsSysWindow{});
    manager.set(window, ToShow{});
    manager.set(window, ScreenPosition(200, 100));
    manager.set(window, Size<int>(400, 300));
    manager.set(window, Color(0, 0, 0.5));
    ecs::Entity menu = manager.createEntity();
    manager.set(menu, NeedsSysWindow{});
    manager.set(menu, IsToolWindow{});
    manager.set(menu, LocalPosition(100, 50));
    manager.set(menu, Size<int>(100, 500));
    manager.set(menu, Color(0, 0.5, 0));
    manager.set(menu, ToShow{});
    
    manager.set(window, Children{{menu}});
    manager.set(menu, Parent{window});

    SysWindowSystem sysWindowSystem;
    locateWindows(manager, window);
    sysWindowSystem.run(manager);
    guiQueue.setTarget(manager.get<SysWindowLink>(window).sysWindow->getHWND());
    std::thread bumper(
        threadProc,
        std::ref(messageQueue),
        std::ref(guiQueue));

    while (true) {
        locateWindows(manager, window);
        sysWindowSystem.run(manager);

        MSG msg;
        GetMessage(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    messageQueue.push(false);
    bumper.join();

    std::cout << "end" << std::endl;

    return 0;
}
