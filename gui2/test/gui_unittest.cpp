// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <cstdint>
#include <format>
#include <tuple>

#include <windows.h>

#include "mock_api.hpp"

namespace {

struct WindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

using WindowMessageHandler =
    std::move_only_function<LRESULT(WindowMessage) noexcept>;

template<typename API>
LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    if (auto* handler =
            API::template getUserPointer<WindowMessageHandler>(hWnd)
    ) {
        return (*handler)(WindowMessage(hWnd, msg, wParam, lParam));
    }
    return API::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool operator==(const WindowMessage& a, const WindowMessage& b) {
    return a.hWnd == b.hWnd
        && a.msg == b.msg
        && a.wParam == b.wParam
        && a.lParam == b.lParam;
}

}  // namespace

TEST_CASE("GUI - windowProc", "[unit][gui]") {
    MockWinAPI::reset();
    HWND hWnd = reinterpret_cast<HWND>(1);
    UINT msg = WM_SIZE;
    WPARAM wParam = SIZE_MINIMIZED;
    LPARAM lParam = MAKELPARAM(11, 12);
    REQUIRE(MockWinAPI::log.empty());
    REQUIRE(windowProc<MockWinAPI>(hWnd, msg, wParam, lParam) == 0);
    REQUIRE(MockWinAPI::log == std::vector<std::string>{
        std::format(
            "DefWindowProc({:#x}, {:#x}, {:#x}, {:#x})",
            reinterpret_cast<uintptr_t>(hWnd), msg, wParam, lParam)
    });
    MockWinAPI::log.clear();
    WindowMessage fnArgs{nullptr, 0, 0, 0};
    WindowMessageHandler fn([&fnArgs](WindowMessage msg) noexcept -> LRESULT {
        fnArgs = msg;
        return 42;
    });
    MockWinAPI::setUserPointer(hWnd, &fn);
    REQUIRE(windowProc<MockWinAPI>(hWnd, msg, wParam, lParam) == 42);
    REQUIRE(MockWinAPI::log.empty());
    REQUIRE(fnArgs == WindowMessage{hWnd, msg, wParam, lParam});
}
