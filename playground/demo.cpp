#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <map>
#include <iostream>
#include <cmath>

#include <gui/core/tools.hpp>
#include <gui/core/widget.hpp>
#include <gui/core/screen.hpp>
#include <gui/core/dispatcher.hpp>
#include <gui/winapi/window.hpp>


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

private:
    std::map<Window*, std::unique_ptr<SysWindow>> wsMap;
    std::map<SysWindow*, Window*> swMap;
};


int main() {
    WindowManager windowManager;
    Window window1;
    Window window2;
    window1.setSize({400, 300});
    window2.setSize({500, 200});
    windowManager.update(window1, {300, 200});
    windowManager.update(window2, {400, 100});

    while (true) {
        MSG msg;
        GetMessageW(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
