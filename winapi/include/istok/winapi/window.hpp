// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>
#include <windows.h>

#include <istok/logging.hpp>

#include "istok/winapi/messages.hpp"


namespace Istok::WinAPI {

template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};


class WndHandle {
public:
    WndHandle() = default;
    WndHandle(Rect<int> screenLocation);
    ~WndHandle();

    WndHandle(const WndHandle&) = delete;
    WndHandle& operator=(const WndHandle&) = delete;
    WndHandle(WndHandle&& source);
    WndHandle& operator=(WndHandle&& source);

    operator bool() const noexcept { return !!hWnd_; }
    HWND getHWnd() const { return hWnd_; }

    void setMessageHandler(WindowMessageHandler&& handler);
    void resetMessageHandler();

private:
    CLASS_WITH_LOGGER_PREFIX("WinAPI", "WinAPI: ");
    HWND hWnd_;
    std::unique_ptr<WindowMessageHandler> handler_;

    void takeFrom(WndHandle& source);
    void clean();
};

}  // namespace Istok::WinAPI
