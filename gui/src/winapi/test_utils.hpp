// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "winapi/base/delegate.hpp"
#include "winapi/base/dispatcher.hpp"
#include "winapi/base/message.hpp"

namespace Istok::GUI::WinAPI {

inline bool operator==(const MSG& a, const MSG& b) {
    return a.hwnd == b.hwnd
        && a.message == b.message
        && a.wParam == b.wParam
        && a.time == b.time
        && a.pt.x == b.pt.x
        && a.pt.y == b.pt.y;
}

inline bool operator==(const Rect<int>& a, const Rect<int>& b) {
    return a.left == b.left
        && a.top == b.top
        && a.right == b.right
        && a.bottom == b.bottom;
}

inline bool operator==(const WindowMessage& a, const WindowMessage& b) {
    return a.hWnd == b.hWnd
        && a.msg == b.msg
        && a.wParam == b.wParam
        && a.lParam == b.lParam;
}

inline bool operator==(
    const WindowEntityMessage& a, const WindowEntityMessage& b
) {
    return a.entity == b.entity
        && a.wParam == b.wParam
        && a.lParam == b.lParam;
}


class MockWinAPI : public trompeloeil::mock_interface<WinAPIDelegate> {
public:
    MAKE_MOCK1(createWindow, HWND(const Rect<int>&), noexcept);
    MAKE_MOCK1(destroyWindow, void(HWND), noexcept);
    MAKE_MOCK1(defWindowProc, LRESULT(const WindowMessage&), noexcept);
    MAKE_MOCK2(setWindowMessageHandler,
        void(HWND, WindowMessageHandler*), noexcept);
    MAKE_MOCK0(getMessage, MSG(), noexcept);
    MAKE_MOCK1(dispatchMessage, void(const MSG& msg), noexcept);
    MAKE_MOCK1(showWindow, void(HWND), noexcept);
};

MockWinAPI& setupMockWinAPI(ECS::ECSManager& ecs, ECS::Entity master);
MockWinAPI& setupMockWinAPI(ECS::ECSManager& ecs);


#define REQUIRE_CREATE_WINDOW(winapi, rect, hWnd) \
    REQUIRE_CALL(winapi, createWindow(rect)).RETURN(hWnd); \
    REQUIRE_CALL(winapi, setWindowMessageHandler(hWnd, _));

#define REQUIRE_CREATE_WINDOW_HANDLER(winapi, rect, hWnd, handlerPtr) \
    REQUIRE_CALL(winapi, createWindow(rect)).RETURN(hWnd); \
    REQUIRE_CALL(winapi, setWindowMessageHandler(hWnd, _)) \
        .LR_SIDE_EFFECT(handlerPtr = _2);

#define ALLOW_DESTROY_WINDOWS(winapi) \
    ALLOW_CALL(winapi, setWindowMessageHandler(_, _)); \
    ALLOW_CALL(winapi, destroyWindow(_));

}  // namespace Istok::GUI::WinAPI
