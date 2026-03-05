// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>

#include <windows.h>

#include "mock_delegate.hpp"

using namespace Istok::GUI::WinAPI;


namespace {

LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    if (auto* delegate = reinterpret_cast<WinAPIDelegate*>(
        GetWindowLongPtr(hWnd, GWLP_USERDATA))
    ) {
        return delegate->windowProc(WindowMessage(hWnd, msg, wParam, lParam));
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

}  // namespace


TEST_CASE("GUI - windowProc", "[unit][gui]") {
    HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
    LPCWSTR className = L"test";
    WNDCLASS wc{0};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    RegisterClass(&wc);
    HWND hWnd = CreateWindow(
        className, L"", NULL, 0, 0, 0, 0,
        nullptr, nullptr, hInstance, nullptr);
    REQUIRE(hWnd);
    MockDelegate delegate;
    SetWindowLongPtr(
        hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&delegate));
    {
        WindowMessage m{hWnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(5, 7)};
        REQUIRE_CALL(delegate, windowProc(m)).RETURN(42);
        REQUIRE(windowProc(m.hWnd, m.msg, m.wParam, m.lParam) == 42);
    }
}
