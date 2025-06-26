// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "tools.hpp"
#include "widget.hpp"

#include <string>

class Window: public Widget {};

template <typename T>
concept MonitorManager = true;

template <typename T>
concept SysWindowFactory = true;

struct MonitorInfo {
    std::string id;
    Rect<int> location;
    Rect<int> workLocation;
};

template <MonitorManager MM, SysWindowFactory SWF>
class Screen: public ParentWidget<Window> {
public:
    Screen(MM& monitorManager, SWF& sysWindowFactory) {}
};
