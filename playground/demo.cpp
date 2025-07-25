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

struct SysWindowLink {
    SysWindow* sysWindow;
};

int main() {
    ecs::EntityComponentManager manager;
    {
        ecs::Entity window = manager.createEntity();
        manager.insert(window, NeedsSysWindow{});
        manager.insert(window, Position<int>(100, 200));
        manager.insert(window, Size<int>(400, 300));
    }
    {
        ecs::Entity window = manager.createEntity();
        manager.insert(window, NeedsSysWindow{});
        manager.insert(window, Position<int>(600, 300));
        manager.insert(window, Size<int>(300, 200));
    }

    std::vector<std::unique_ptr<SysWindow>> sysWindows;
    for (auto e : manager.view<NeedsSysWindow, Position<int>, Size<int>>()) {
        sysWindows.push_back(
            std::make_unique<SysWindow>(
                "Istok",
                manager.get<Position<int>>(e),
                manager.get<Size<int>>(e)));
        sysWindows.back()->show();
        manager.remove<NeedsSysWindow>(e);
        manager.insert(e, SysWindowLink{sysWindows.back().get()});
    }

    ModernGLContext gl(sysWindows.front()->getDC());

    while (true) {
        MSG msg;
        GetMessageW(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        for (auto e : manager.view<SysWindowLink>()) {
            SysWindow* sysWindow = manager.get<SysWindowLink>(e).sysWindow;
            wglMakeCurrent(sysWindow->getDC(), gl.getGL());
            glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            SwapBuffers(sysWindow->getDC());
        }
    }

    return 0;
}
