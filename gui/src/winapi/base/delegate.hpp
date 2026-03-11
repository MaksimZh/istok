// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>

#include "istok/gui/base.hpp"
#include "winapi/base/message.hpp"


namespace Istok::GUI::WinAPI {

class WinAPIDelegate {
public:
    virtual ~WinAPIDelegate() = default;
    virtual HWND createWindow(const Rect<int>& location) noexcept = 0;
    virtual void destroyWindow(HWND hWnd) noexcept = 0;
    virtual LRESULT defWindowProc(const WindowMessage& message) noexcept = 0;

    virtual void setWindowMessageHandler(
        HWND hWnd, WindowMessageHandler* handler) noexcept = 0;

    virtual void getMessage(MSG& msg) noexcept = 0;
    virtual void dispatchMessage(const MSG& msg) noexcept = 0;

    virtual void showWindow(HWND hWnd) noexcept = 0;
};

}  // namespace Istok::GUI::WinAPI
