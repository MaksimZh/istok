// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "winapi/base/delegate.hpp"
#include "winapi/base/message.hpp"

namespace Istok::GUI::WinAPI {

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

class MockWinAPI : public trompeloeil::mock_interface<WinAPIDelegate> {
public:
    MAKE_MOCK1(createWindow, HWND(const Rect<int>&), noexcept);
    MAKE_MOCK1(destroyWindow, void(HWND), noexcept);
    MAKE_MOCK1(defWindowProc, LRESULT(const WindowMessage&), noexcept);
    MAKE_MOCK2(setRawUserPointer, void(HWND, LONG_PTR), noexcept);
    MAKE_MOCK1(getMessage, void(MSG&), noexcept);
    MAKE_MOCK1(dispatchMessage, void(const MSG& msg), noexcept);
    MAKE_MOCK1(showWindow, void(HWND), noexcept);
};

}  // namespace Istok::GUI::WinAPI
