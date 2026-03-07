// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <set>

#include <istok/logging.hpp>

#include "message.hpp"


namespace Istok::GUI::WinAPI::internal {

void logWindowProc(const WindowMessage& message) {
    static std::set<UINT> mouseMessages = {
        WM_ENTERIDLE,
        WM_SETCURSOR,
        WM_MOUSEMOVE,
        WM_NCHITTEST,
        WM_NCMOUSEMOVE,
        WM_NCMOUSELEAVE,
    };
    if (mouseMessages.contains(message.msg)) {
        WITH_LOGGER_PREFIX("Istok.GUI.WinAPI.WndProc.MouseMove", "WndProc: ");
        LOG_TRACE("{}", message);
    } else {
        WITH_LOGGER_PREFIX("Istok.GUI.WinAPI.WndProc", "WndProc: ");
        LOG_TRACE("{}", message);
    }
}

}  // namespace Istok::GUI::WinAPI::internal
