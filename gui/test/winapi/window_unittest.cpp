// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "src/winapi/base/window.hpp"

#include <windows.h>

#include "test/winapi/utils.hpp"

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


TEST_CASE("Window - empty", "[unit][winapi]") {
    Window window;
    REQUIRE(window.getHWnd() == nullptr);
}


TEST_CASE("Window - lifecycle", "[unit][winapi]") {
    MockWinAPI winapi;
    const Rect<int> location{2, 3, 4, 5};
    const HWND hWnd = reinterpret_cast<HWND>(1);
    const LRESULT handlerResult = 42;
    WindowMessageHandler handler(
        [](WindowMessage message) noexcept { return handlerResult; });

    Window* window;
    WindowMessageHandler* storedHandler = nullptr;
    {
        REQUIRE_CALL(winapi, createWindow(location)).RETURN(hWnd);
        REQUIRE_CALL(winapi, setWindowMessageHandler(hWnd, _))
            .LR_SIDE_EFFECT(storedHandler = _2);
        window = new Window(winapi, location, std::move(handler));
    }
    REQUIRE(window->getHWnd() == hWnd);
    REQUIRE(storedHandler);
    REQUIRE((*storedHandler)(WindowMessage{}) == handlerResult);
    {
        REQUIRE_CALL(winapi, setWindowMessageHandler(hWnd, nullptr));
        REQUIRE_CALL(winapi, destroyWindow(hWnd));
        delete window;
    }
}
