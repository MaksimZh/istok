// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <map>

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <windows.h>

#include "istok/gui/base.hpp"
#include "winapi/base/message.hpp"
#include "winapi/base/winapi_delegate.hpp"


namespace Istok::GUI::WinAPI {

class FakeWindowsMockWinAPI : virtual public WinAPIDelegate {
public:
    HWND createWindow(const Rect<int>& location) noexcept override;
    void destroyWindow(HWND hWnd) noexcept override;
    void setWindowMessageHandler(
        HWND hWnd, WindowMessageHandler* handler) noexcept override;
    size_t windowsCount() const noexcept;
    WindowMessageHandler* getWindowMessageHandler(HWND hWnd) noexcept;

    MAKE_MOCK1(defWindowProc, LRESULT(const WindowMessage&), noexcept);
    MAKE_MOCK0(getMessage, MSG(), noexcept);
    MAKE_MOCK1(dispatchMessage, void(const MSG& msg), noexcept);
    MAKE_MOCK1(showWindow, void(HWND), noexcept);

private:
    size_t counter_ = 0;
    std::map<HWND, WindowMessageHandler*> handlers_;
};

}  // namespace Istok::GUI::WinAPI