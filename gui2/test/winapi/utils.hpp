// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "winapi/delegate.hpp"
#include "winapi/message.hpp"

namespace Istok::GUI::WinAPI {

inline bool operator==(const WindowMessage& a, const WindowMessage& b) {
    return a.hWnd == b.hWnd
        && a.msg == b.msg
        && a.wParam == b.wParam
        && a.lParam == b.lParam;
}

class MockWinAPI : public trompeloeil::mock_interface<WinAPIDelegate> {
public:
    MAKE_MOCK1(defWindowProc, LRESULT(const WindowMessage& message), noexcept);
};

}  // namespace Istok::GUI::WinAPI
