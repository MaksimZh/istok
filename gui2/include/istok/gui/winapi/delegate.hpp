// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>


namespace Istok::GUI::WinAPI {

struct WindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

class WinAPIDelegate {
public:
    virtual ~WinAPIDelegate() = default;
    virtual LRESULT windowProc(WindowMessage message) noexcept = 0;
    virtual LRESULT defWindowProc(WindowMessage message) noexcept = 0;
};

}  // namespace Istok::GUI::WinAPI
