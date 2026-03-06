// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <concepts>
#include <cstdint>
#include <format>
#include <functional>
#include <string>

#include <windows.h>


namespace Istok::GUI::WinAPI {

struct WindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

std::string toString(const WindowMessage& message);

using WindowMessageHandler =
    std::move_only_function<LRESULT(const WindowMessage&) noexcept>;

template<typename T>
concept WindowMessageHandlerStorage = requires(HWND hWnd) {
    {T::get(hWnd)} noexcept -> std::same_as<WindowMessageHandler&>;
};

}  // namespace Istok::GUI::WinAPI


template <>
struct std::formatter<HWND> : std::formatter<std::string> {
    auto format(HWND hWnd, std::format_context& ctx) const {
        return std::formatter<std::string>::format(
            std::format("HWND({:#x})", reinterpret_cast<uintptr_t>(hWnd)),
            ctx);
    }
};

template <>
struct std::formatter<Istok::GUI::WinAPI::WindowMessage> :
    std::formatter<std::string>
{
    auto format(
        const Istok::GUI::WinAPI::WindowMessage& message,
        std::format_context& ctx
    ) const {
        return std::formatter<std::string>::format(
            Istok::GUI::WinAPI::toString(message),
            ctx);
    }
};
