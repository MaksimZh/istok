// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>

namespace Istok::GUI::WinAPI {

#define REQUIRE_CREATE_WINDOW(winapi, rect, hWnd) \
    REQUIRE_CALL(winapi, createWindow(rect)).RETURN(hWnd); \
    REQUIRE_CALL(winapi, setWindowMessageHandler(hWnd, _));

#define REQUIRE_CREATE_WINDOW_HANDLER(winapi, rect, hWnd, handlerPtr) \
    REQUIRE_CALL(winapi, createWindow(rect)).RETURN(hWnd); \
    REQUIRE_CALL(winapi, setWindowMessageHandler(hWnd, _)) \
        .LR_SIDE_EFFECT(handlerPtr = _2);

#define REQUIRE_DESTROY_WINDOW(winapi, hWnd) \
    REQUIRE_CALL(winapi, setWindowMessageHandler(hWnd, nullptr)); \
    REQUIRE_CALL(winapi, destroyWindow(hWnd));

#define ALLOW_DESTROY_WINDOWS(winapi) \
    ALLOW_CALL(winapi, setWindowMessageHandler(_, _)); \
    ALLOW_CALL(winapi, destroyWindow(_));

}  // namespace Istok::GUI::WinAPI
