// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <windows.h>

#include "message.hpp"


namespace Istok::GUI::WinAPI {

class WinAPIDelegate {
public:
    virtual ~WinAPIDelegate() = default;
    virtual LRESULT defWindowProc(const WindowMessage& message) noexcept = 0;
};

}  // namespace Istok::GUI::WinAPI
