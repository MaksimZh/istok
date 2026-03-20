// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "winapi/base/dispatcher.hpp"
#include "winapi/base/message.hpp"
#include "winapi/base/winapi_delegate.hpp"


namespace Istok::GUI {
inline bool operator==(const Rect<int>& a, const Rect<int>& b) {
    return a.left == b.left
        && a.top == b.top
        && a.right == b.right
        && a.bottom == b.bottom;
}
}  // namespace Istok::GUI

namespace Istok::GUI::WinAPI {

inline bool operator==(const MSG& a, const MSG& b) {
    return a.hwnd == b.hwnd
        && a.message == b.message
        && a.wParam == b.wParam
        && a.time == b.time
        && a.pt.x == b.pt.x
        && a.pt.y == b.pt.y;
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

}  // namespace Istok::GUI::WinAPI
