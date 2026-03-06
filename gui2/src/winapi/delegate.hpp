// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>

#include "istok/gui/geometry.hpp"
#include "message.hpp"


namespace Istok::GUI::WinAPI {

class WinAPIDelegate {
public:
    virtual ~WinAPIDelegate() = default;
    virtual HWND createWindow(const Rect<int>& location) noexcept = 0;
    virtual LRESULT defWindowProc(const WindowMessage& message) noexcept = 0;

    template<typename T>
    void setUserPointer(HWND hWnd, T* ptr) noexcept {
        setRawUserPointer(hWnd, reinterpret_cast<LONG_PTR>(ptr));
    }
    virtual void setRawUserPointer(HWND hWnd, LONG_PTR ptr) noexcept = 0;
};

}  // namespace Istok::GUI::WinAPI
