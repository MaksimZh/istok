// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/screen.hpp>

#include <vector>
#include <memory>


namespace {

    class FakeSysWindow {

    };
    
    
    class FakeSysWindowFactory {

    };

    
    class FakeMonitorManager {
    public:
        FakeMonitorManager(std::vector<MonitorInfo> monitors) 
        : monitors(std::move(monitors)) {}

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
    };
}


TEST_CASE("Screen - ??", "[unit][gui]") {
    FakeSysWindowFactory sysWindowFactory;
    FakeMonitorManager monitorManager(std::vector{
        MonitorInfo{"display", {0, 0, 400, 300}, {0, 0, 400, 280}}
    });
    Screen screen(monitorManager, sysWindowFactory);
}
