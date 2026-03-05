// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "windowproc.hpp"

#include <windows.h>

#include "istok/gui/winapi/delegate.hpp"


namespace Istok::GUI::WinAPI {

LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    if (auto* delegate = reinterpret_cast<WinAPIDelegate*>(
        GetWindowLongPtr(hWnd, GWLP_USERDATA))
    ) {
        return delegate->windowProc(WindowMessage(hWnd, msg, wParam, lParam));
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

}  // namespace Istok::GUI::WinAPI
