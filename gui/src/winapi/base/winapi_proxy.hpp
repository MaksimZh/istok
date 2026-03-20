// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>

#include "istok/gui/base.hpp"
#include "winapi/base/message.hpp"
#include "winapi/base/winapi_delegate.hpp"


namespace Istok::GUI::WinAPI {

class WinAPIProxy : virtual public WinAPIDelegate {
public:
    WinAPIProxy(WinAPIDelegate* delegate) : delegate_(delegate) {}

    HWND createWindow(const Rect<int>& location) noexcept override {
        return delegate_->createWindow(location);
    }

    void destroyWindow(HWND hWnd) noexcept override {
        delegate_->destroyWindow(hWnd);
    }

    LRESULT defWindowProc(const WindowMessage& message) noexcept override {
        return delegate_->defWindowProc(message);
    }

    void setWindowMessageHandler(
        HWND hWnd, WindowMessageHandler* handler
    ) noexcept override {
        delegate_->setWindowMessageHandler(hWnd, handler);
    }

    MSG getMessage() noexcept override {
        return delegate_->getMessage();
    }

    void dispatchMessage(const MSG& msg) noexcept override {
        delegate_->dispatchMessage(msg);
    }

    void showWindow(HWND hWnd) noexcept override {
        delegate_->showWindow(hWnd);
    }

private:
    WinAPIDelegate* delegate_;
};

}  // namespace Istok::GUI::WinAPI