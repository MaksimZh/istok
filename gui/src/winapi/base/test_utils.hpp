// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#define NOMINMAX
#pragma once

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <windows.h>

#include "winapi/base/dispatcher.hpp"
#include "winapi/base/message.hpp"
#include "winapi/base/winapi_delegate.hpp"


namespace Istok::GUI::WinAPI {

inline bool operator==(const MSG& a, const MSG& b) {
    return a.hwnd == b.hwnd
        && a.message == b.message
        && a.wParam == b.wParam
        && a.time == b.time
        && a.pt.x == b.pt.x
        && a.pt.y == b.pt.y;
}

inline bool operator==(const WindowMessage& a, const WindowMessage& b) {
    return a.hWnd == b.hWnd
        && a.msg == b.msg
        && a.wParam == b.wParam
        && a.lParam == b.lParam;
}

inline bool operator==(
    const WindowEntityMessage& a, const WindowEntityMessage& b
) {
    return a.entity == b.entity
        && a.wParam == b.wParam
        && a.lParam == b.lParam;
}


template <typename T, typename... Args>
T& setupUnique(ECS::ECSManager& ecs, ECS::Entity master, Args&&... args) {
    auto container = std::make_unique<T>(std::forward<Args>(args)...);
    T& reference = *container;
    ecs.insert(master, std::move(container));
    return reference;
}

template <typename T, typename... Args>
T& setupWinAPI(ECS::ECSManager& ecs, ECS::Entity master, Args&&... args) {
    auto container = std::make_unique<T>(std::forward<Args>(args)...);
    T& winapi = *container;
    ecs.insert(master, std::unique_ptr<WinAPIDelegate>(std::move(container)));
    return winapi;
}


class MockWinAPI : virtual public WinAPIDelegate {
public:
    MAKE_MOCK1(createWindow, HWND(const Rect<int>&), noexcept);
    MAKE_MOCK1(destroyWindow, void(HWND), noexcept);
    MAKE_MOCK1(defWindowProc, LRESULT(const WindowMessage&), noexcept);
    MAKE_MOCK2(setWindowMessageHandler,
        void(HWND, WindowMessageHandler*), noexcept);
    MAKE_MOCK0(getMessage, MSG(), noexcept);
    MAKE_MOCK1(dispatchMessage, void(const MSG& msg), noexcept);
    MAKE_MOCK1(showWindow, void(HWND), noexcept);
};


class NullWinAPI : virtual public WinAPIDelegate {
public:
    HWND createWindow(const Rect<int>& location) noexcept override {
        return nullptr;
    }

    void destroyWindow(HWND hWnd) noexcept override {}

    LRESULT defWindowProc(const WindowMessage& message) noexcept override {
        return 0;
    }

    void setWindowMessageHandler(
        HWND hWnd, WindowMessageHandler* handler) noexcept override {}

    MSG getMessage() noexcept override {
        return MSG{};
    }

    void dispatchMessage(const MSG& msg) noexcept override {}

    void showWindow(HWND hWnd) noexcept override {}
};


class FakeWindowsMockWinAPI : virtual public WinAPIDelegate {
public:
    HWND createWindow(const Rect<int>& location) noexcept override;
    void destroyWindow(HWND hWnd) noexcept override;
    void setWindowMessageHandler(
        HWND hWnd, WindowMessageHandler* handler) noexcept override;
    size_t windowsCount() const noexcept;
    LRESULT handleMessage(const WindowMessage& message) noexcept;

    MAKE_MOCK1(defWindowProc, LRESULT(const WindowMessage&), noexcept);
    MAKE_MOCK0(getMessage, MSG(), noexcept);
    MAKE_MOCK1(dispatchMessage, void(const MSG& msg), noexcept);
    MAKE_MOCK1(showWindow, void(HWND), noexcept);

private:
    size_t counter_ = 0;
    std::map<HWND, WindowMessageHandler*> handlers_;
};


class WinAPIProxy : virtual public WinAPIDelegate {
public:
    WinAPIProxy(WinAPIDelegate& delegate) : delegate_(delegate) {}

    HWND createWindow(const Rect<int>& location) noexcept override {
        return delegate_.createWindow(location);
    }

    void destroyWindow(HWND hWnd) noexcept override {
        delegate_.destroyWindow(hWnd);
    }

    LRESULT defWindowProc(const WindowMessage& message) noexcept override {
        return delegate_.defWindowProc(message);
    }

    void setWindowMessageHandler(
        HWND hWnd, WindowMessageHandler* handler
    ) noexcept override {
        delegate_.setWindowMessageHandler(hWnd, handler);
    }

    MSG getMessage() noexcept override {
        return delegate_.getMessage();
    }

    void dispatchMessage(const MSG& msg) noexcept override {
        delegate_.dispatchMessage(msg);
    }

    void showWindow(HWND hWnd) noexcept override {
        delegate_.showWindow(hWnd);
    }

private:
    WinAPIDelegate& delegate_;
};

void setupWinAPIProxy(
    ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& target);

}  // namespace Istok::GUI::WinAPI
