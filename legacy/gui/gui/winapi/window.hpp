// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <logging.hpp>

#include <memory>

#include <windows.h>

namespace Istok::GUI::WinAPI {

template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};


struct WindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;

    LRESULT handleByDefault() noexcept;
};


class WindowMessageHandler {
public:
    virtual ~WindowMessageHandler() = default;
    virtual LRESULT handleWindowMessage(WindowMessage message) noexcept = 0;
};


class WndHandle {
public:
    WndHandle() = default;
    WndHandle(Rect<int> screenLocation);
    ~WndHandle();

    WndHandle(const WndHandle&) = delete;
    WndHandle& operator=(const WndHandle&) = delete;
    WndHandle(WndHandle&& source) = default;
    WndHandle& operator=(WndHandle&& source) = default;

    operator bool() const noexcept {
        return hWnd_ && *hWnd_;
    }

    HWND getHWnd() const {
        return hWnd_ ? *hWnd_ : nullptr;
    }

    void setHandler(WindowMessageHandler* handler);
    void setVisibility(bool value);

private:
    CLASS_WITH_LOGGER_PREFIX("Windows", "WinAPI: ");
    
    std::unique_ptr<HWND> hWnd_;
};


class DCHandle {
public:
    DCHandle() = default;
    DCHandle(HWND hWnd);
    ~DCHandle();

    DCHandle(const DCHandle&) = delete;
    DCHandle& operator=(const DCHandle&) = delete;
    DCHandle(DCHandle&&) = default;
    DCHandle& operator=(DCHandle&& other) = default;

    operator bool() const noexcept {
        return hDC_ && *hDC_;
    }

    HDC getDC() const noexcept {
        return hDC_ ? *hDC_ : nullptr;
    }

private:
    std::unique_ptr<HDC> hDC_;
};


} // namespace Istok::GUI::WinAPI
