// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <concepts>
#include <functional>

#include <windows.h>


namespace Istok::GUI::WinAPI {

struct WindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

using WindowMessageHandler =
    std::move_only_function<LRESULT(const WindowMessage&) noexcept>;

template<typename T>
concept WindowMessageHandlerStorage = requires(HWND hWnd) {
    {T::get(hWnd)} noexcept -> std::same_as<WindowMessageHandler&>;
};

}  // namespace Istok::GUI::WinAPI
