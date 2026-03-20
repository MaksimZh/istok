// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "winapi/base/fake_windows.hpp"

#include <windows.h>

#include "istok/gui/base.hpp"
#include "winapi/base/message.hpp"


namespace Istok::GUI::WinAPI {

HWND FakeWindowsMockWinAPI::createWindow(const Rect<int>& location) noexcept {
    HWND hWnd = reinterpret_cast<HWND>(++counter_);
    handlers_[hWnd] = nullptr;
    return hWnd;
}

void FakeWindowsMockWinAPI::destroyWindow(HWND hWnd) noexcept {
    handlers_.erase(hWnd);
}

void FakeWindowsMockWinAPI::setWindowMessageHandler(
    HWND hWnd, WindowMessageHandler* handler
) noexcept {
    handlers_[hWnd] = handler;
}

size_t FakeWindowsMockWinAPI::windowsCount() const noexcept {
    return handlers_.size();
}

WindowMessageHandler* FakeWindowsMockWinAPI::getWindowMessageHandler(
    HWND hWnd
) noexcept {
    return handlers_[hWnd];
}

}  // namespace Istok::GUI::WinAPI
