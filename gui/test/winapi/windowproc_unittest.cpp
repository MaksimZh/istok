// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/windowproc.hpp"

#include <windows.h>

#include "utils.hpp"

using namespace Istok::GUI::WinAPI;


namespace {

struct MockHandlerStorageInstance {
    MAKE_MOCK1(get, WindowMessageHandler&(HWND hWnd), noexcept);
};

struct MockHandlerStorage {
    inline static thread_local MockHandlerStorageInstance instance;
    static WindowMessageHandler& get(HWND hWnd) noexcept {
        return instance.get(hWnd);
    }
};

struct MockHandlerChecker {
    MAKE_MOCK1(call, LRESULT(const WindowMessage&), noexcept);
};

}  // namespace


TEST_CASE("WindowProc - handler", "[unit][winapi]") {
    const WindowMessage message{
        reinterpret_cast<HWND>(1), WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(5, 7)};
    MockHandlerChecker checker;
    WindowMessageHandler handler(
        [&checker](WindowMessage m) noexcept { return checker.call(m); });
    {
        const LRESULT handlerReturn = 42;
        REQUIRE_CALL(MockHandlerStorage::instance, get(message.hWnd))
            .LR_RETURN(handler);
        REQUIRE_CALL(checker, call(message)).RETURN(handlerReturn);
        REQUIRE(windowProc<MockHandlerStorage>(
            message.hWnd, message.msg, message.wParam, message.lParam)
            == handlerReturn);
    }
}
