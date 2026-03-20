// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#define NOMINMAX
#include "winapi/base/window.hpp"

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <windows.h>

#include "istok/gui/base.hpp"
#include "winapi/base/test_utils.hpp"

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


namespace {

struct MockHandler {
    MAKE_MOCK1(call, LRESULT(const WindowMessage&), noexcept);
};

}  // namespace


TEST_CASE("Window - empty", "[unit][winapi]") {
    Window window;
    REQUIRE(window.getHWnd() == nullptr);
}


TEST_CASE("Window - lifecycle", "[unit][winapi]") {
    MockWinAPI winapi;
    const Rect<int> location{2, 3, 4, 5};
    const HWND hWnd = reinterpret_cast<HWND>(1);
    const WindowMessage message{hWnd, WM_CLOSE, 0, 0};
    const LRESULT handlerResult = 42;
    MockHandler handler;

    Window* window;
    WindowMessageHandler* storedHandler = nullptr;
    {
        REQUIRE_CALL(winapi, createWindow(location)).RETURN(hWnd);
        REQUIRE_CALL(winapi, setWindowMessageHandler(hWnd, _))
            .LR_SIDE_EFFECT(storedHandler = _2);
        window = new Window(
            winapi, location,
            std::move([&handler](const WindowMessage& message) noexcept {
                return handler.call(message); }));
    }
    REQUIRE(window->getHWnd() == hWnd);
    REQUIRE(storedHandler);

    {
        REQUIRE_CALL(handler, call(message)).RETURN(handlerResult);
        REQUIRE((*storedHandler)(message) == handlerResult);
    }

    {
        REQUIRE_CALL(winapi, setWindowMessageHandler(hWnd, nullptr));
        REQUIRE_CALL(winapi, destroyWindow(hWnd));
        delete window;
    }
}
