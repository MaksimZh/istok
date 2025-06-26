#include <string>
#include <optional>
#include <memory>
#include <map>
#include <iostream>

#include <gui/core/tools.hpp>
#include <gui/core/widget.hpp>
#include <gui/winapi/window.hpp>


BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, &monitorInfo);

    wprintf(L"Device: %s\n", monitorInfo.szDevice);
    printf("    rect: %d %d %d %d\n",
        monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
        monitorInfo.rcMonitor.right, monitorInfo.rcMonitor.bottom);
    printf("    work: %d %d %d %d\n",
        monitorInfo.rcWork.left, monitorInfo.rcWork.top,
        monitorInfo.rcWork.right, monitorInfo.rcWork.bottom);
    printf("    primary: %d\n", monitorInfo.dwFlags & MONITORINFOF_PRIMARY);
    return TRUE; // Продолжаем перечисление
}


int main() {
    int r = EnumDisplayMonitors(GetDC(NULL), NULL, MonitorEnumProc, 0);

/*
    Screen screen;
    screen.add("main", std::make_unique<Window>());

    while (true) {
        MSG msg;
        GetMessageW(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
*/
    return 0;
}
