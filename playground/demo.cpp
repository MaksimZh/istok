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


int main() {
    Window w;
    w.setSize({400, 300});
    Position<float> pos{300, 200}; 
    Size<float> size = w.getSize();
    
    Position<int> sysPos{
        static_cast<int>(std::round(pos.x)),
        static_cast<int>(std::round(pos.y))
    };
    Size<int> sysSize{
        static_cast<int>(std::round(size.width)),
        static_cast<int>(std::round(size.height))
    };
    SysWindow s("Istok", sysPos, sysSize);
    s.show();

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
