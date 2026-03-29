// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>

#include <windows.h>

#include <istok/logging.hpp>

#include "istok/gui/base.hpp"
#include "winapi/core2/message.hpp"


namespace Istok::GUI::WinAPI {

using WindowMessageHandler =
    std::move_only_function<LRESULT(const WindowMessage&) noexcept>;


class Window {
public:
    Window() = default;
    Window(Rect<int> screenLocation, WindowMessageHandler&& handler) noexcept;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& source);
    Window& operator=(Window&& source);

    HWND getHWnd() const { return hWnd_; }

private:
    CLASS_WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    HWND hWnd_ = nullptr;
    std::unique_ptr<WindowMessageHandler> handler_;

    void takeFrom(Window& source);
    void clear();
};

}  // namespace Istok::GUI::WinAPI