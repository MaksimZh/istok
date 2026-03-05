// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <format>
#include <functional>
#include <string>
#include <windows.h>

namespace Istok::WinAPI {

struct WindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

using WindowMessageHandler =
    std::move_only_function<LRESULT(WindowMessage) noexcept>;

template <typename T>
requires std::is_pointer_v<T>
std::string toString(T x) noexcept {
    return std::format("{:#x}", reinterpret_cast<uintptr_t>(x));
}

std::string toString(LPCWSTR w) noexcept;

std::string formatMessage(const WindowMessage& message) noexcept;
LRESULT handleMessageByDefault(const WindowMessage& message) noexcept;

LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

}  // namespace Istok::WinAPI
