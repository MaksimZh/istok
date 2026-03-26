// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#define NOMINMAX
#include "winapi/base/window2.hpp"

#include <functional>
#include <memory>
#include <utility>

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <windows.h>

#include "istok/gui/base.hpp"
#include "winapi/base/test_utils.hpp"

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


namespace {

// TODO: move to `functional` package
using Closure = std::move_only_function<void() noexcept>;

// TODO: move to `testing` package
struct DeathWatch {
    Closure callback_;
    DeathWatch(Closure&& callback) : callback_(std::move(callback)) {}
    ~DeathWatch() { callback_(); }
};

struct MockHandler {
    MAKE_MOCK1(run, LRESULT(const WindowMessage&), noexcept);
    MAKE_MOCK0(kill, void(), noexcept);

    WindowMessageHandler get() noexcept {
        auto dw = std::make_unique<DeathWatch>([this]() noexcept { kill(); });
        return [this, x=std::move(dw)](
            const WindowMessage& message
        ) noexcept -> LRESULT {
            return run(message);
        };
    }
};

}  // namespace


TEST_CASE("Window - empty", "[unit][winapi]") {
    Window2 window;
    REQUIRE_FALSE(window.getHWnd());
}


TEST_CASE("Window - real", "[unit][winapi]") {
    const Rect<int> location{1, 2, 301, 302};
    MockHandler handler;

    auto window = std::make_unique<Window2>(location, handler.get());
    HWND hWnd = window->getHWnd();
    REQUIRE(hWnd);
    REQUIRE(IsWindow(hWnd));
    RECT rect;
    REQUIRE(GetWindowRect(hWnd, &rect));
    REQUIRE(
        Rect<int>{rect.left, rect.top, rect.right, rect.bottom} == location);

    const WindowMessage message{
        hWnd, WM_SETCURSOR, 2, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE)};
    {
        REQUIRE_CALL(handler, run(message)).RETURN(42);
        REQUIRE(
            SendMessage(
                message.hWnd, message.msg, message.wParam, message.lParam)
            == 42);
    }

    {
        REQUIRE_CALL(handler, kill());
        window.reset();
    }
    REQUIRE_FALSE(IsWindow(hWnd));
}
