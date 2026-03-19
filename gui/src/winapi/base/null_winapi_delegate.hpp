// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>

#include "istok/gui/base.hpp"
#include "winapi/base/message.hpp"
#include "winapi/base/winapi_delegate.hpp"


namespace Istok::GUI::WinAPI {

class NullWinAPIDelegate final : public WinAPIDelegate {
public:
    HWND createWindow(const Rect<int>& location) noexcept override {
        return nullptr;
    }

    void destroyWindow(HWND hWnd) noexcept override {}

    LRESULT defWindowProc(const WindowMessage& message) noexcept override {
        return 0;
    }

    void setWindowMessageHandler(
        HWND hWnd, WindowMessageHandler* handler) noexcept override {}

    MSG getMessage() noexcept override {
        return MSG{};
    }

    void dispatchMessage(const MSG& msg) noexcept override {}

    void showWindow(HWND hWnd) noexcept override {}
};

}  // namespace Istok::GUI::WinAPI