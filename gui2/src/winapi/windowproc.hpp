// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>

#include "message.hpp"

namespace Istok::GUI::WinAPI {

template<WindowMessageHandlerStorage HandlerStorage>
LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    return HandlerStorage::get(hWnd)(WindowMessage{hWnd, msg, wParam, lParam});
}

}  // namespace Istok::GUI::WinAPI
