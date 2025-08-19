// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/winapi/platform.hpp>
#include <gui/common/message.hpp>
#include <gui/common/core.hpp>

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;

TEST_CASE("WinAPI - Platform", "[unit][gui]") {
    using Platform = WinPlatform<int>;
    auto appQueue = std::make_shared<AppQueue<int>>();
    GUICore<Platform> core(appQueue);
}
