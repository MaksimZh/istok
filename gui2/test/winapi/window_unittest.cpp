// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/window.hpp"

#include <windows.h>

#include "utils.hpp"

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;

namespace {

bool operator==(const Rect<int>& a, const Rect<int>& b) {
    return a.left == b.left
        && a.top == b.top
        && a.right == b.right
        && a.bottom == b.bottom;
}

}

TEST_CASE("Window - setup", "[unit][winapi]") {
    MockWinAPI winapi;
    const Rect<int> location{2, 3, 4, 5};
    const HWND hWnd = reinterpret_cast<HWND>(1);

    Window window;
    REQUIRE(window.getHWnd() == nullptr);

    {
        REQUIRE_CALL(winapi, createWindow(location)).RETURN(hWnd);
        window = Window(winapi, location);
        REQUIRE(window.getHWnd() == hWnd);
    }

    const LRESULT handlerResult = 42;
    WindowMessageHandler handler(
        [](WindowMessage message) noexcept { return handlerResult; });
    {
        WindowMessageHandler* storedHandler = nullptr;
        REQUIRE_CALL(winapi, setRawUserPointer(hWnd, _))
            .LR_SIDE_EFFECT(
                storedHandler = reinterpret_cast<WindowMessageHandler*>(_2));
        window.setMessageHandler(std::move(handler));
        REQUIRE(storedHandler);
        REQUIRE((*storedHandler)(WindowMessage{}) == handlerResult);
    }
}
