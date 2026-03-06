// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>

#include "message.hpp"

namespace Istok::GUI::WinAPI {

namespace internal {
void logWindowProc(const WindowMessage& message);
}

// Loggers:
//     WinAPI.WndProc
//     WinAPI.WndProc.MouseMove
template<WindowMessageHandlerStorage HandlerStorage>
LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    WindowMessage message{hWnd, msg, wParam, lParam};
    internal::logWindowProc(message);
    return HandlerStorage::get(hWnd)(message);
}

}  // namespace Istok::GUI::WinAPI
