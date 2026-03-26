// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>

#include <windows.h>

#include <istok/logging.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/message.hpp"


namespace Istok::GUI::WinAPI {


class Window2 {
public:
    Window2() = default;
    Window2(Rect<int> screenLocation, WindowMessageHandler&& handler) noexcept;
    ~Window2();

    Window2(const Window2&) = delete;
    Window2& operator=(const Window2&) = delete;
    Window2(Window2&& source);
    Window2& operator=(Window2&& source);

    HWND getHWnd() const { return hWnd_; }

private:
    CLASS_WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    HWND hWnd_ = nullptr;
    std::unique_ptr<WindowMessageHandler> handler_;

    void takeFrom(Window2& source);
    void clear();
};

}  // namespace Istok::WinAPI
