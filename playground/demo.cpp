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

struct Floating {};

int main() {
    ecs::EntityComponentManager manager;
    std::cout << "Hello!" << std::endl;
    return 0;
}
