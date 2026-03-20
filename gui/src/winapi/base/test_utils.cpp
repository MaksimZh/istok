// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "winapi/base/test_utils.hpp"

#include <istok/ecs.hpp>

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

LRESULT FakeWindowsMockWinAPI::handleMessage(
    const WindowMessage& message
) noexcept {
    auto it = handlers_.find(message.hWnd);
    if (it == handlers_.end()) {
        return defWindowProc(message);
    }
    return (*it->second)(message);
}


void setupWinAPIProxy(
    ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& target
) {
    setupWinAPI<WinAPIProxy>(ecs, master, target);
}

}  // namespace Istok::GUI::WinAPI
