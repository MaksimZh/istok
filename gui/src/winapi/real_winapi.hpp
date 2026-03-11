// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>

#include "base/delegate.hpp"


namespace Istok::GUI::WinAPI {

class RealWinAPI : public WinAPIDelegate {
public:
    HWND createWindow(const Rect<int>& location) noexcept override;
    void destroyWindow(HWND hWnd) noexcept override;
    LRESULT defWindowProc(const WindowMessage& message) noexcept override;
    virtual void setWindowMessageHandler(
        HWND hWnd, WindowMessageHandler* handler) noexcept override;
    void getMessage(MSG& msg) noexcept override;
    void dispatchMessage(const MSG& msg) noexcept override;
    void showWindow(HWND hWnd) noexcept override;
};

}  // namespace Istok::GUI::WinAPI
