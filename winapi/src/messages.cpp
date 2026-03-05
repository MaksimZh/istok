// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "istok/winapi/messages.hpp"

#include <format>
#include <set>
#include <string>
#include <windows.h>
#include <windowsx.h>

#include <istok/logging.hpp>

namespace Istok::WinAPI {

namespace {

std::string formatAsWINDOWPOS(LPARAM lParam) {
    auto wp = reinterpret_cast<WINDOWPOS*>(lParam);
    return std::format(
        "{{[{}], ({:d}, {:d}), ({:d} x {:d}), {:#x}}}",
        toString(wp->hwndInsertAfter),
        wp->x, wp->y, wp->cx, wp->cy, wp->flags);
}

std::string formatAsPOINTS(LPARAM lParam) {
    return std::format(
        "({:d}, {:d})",
        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
}

std::string formatAsRECT(LPARAM lParam) {
    auto rect = reinterpret_cast<RECT*>(lParam);
    return std::format(
        "{{{:d}, {:d}, {:d}, {:d}}}",
        rect->left, rect->top, rect->right, rect->bottom);
}


std::string formatMessage(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    const std::string prefix = std::format("[{}].", toString(hWnd));
    switch (msg) {
    case 0x0001:
        return std::format("{}WM_CREATE(\"{}\")", prefix,
            toString(reinterpret_cast<CREATESTRUCT*>(lParam)->lpszName));
    case 0x0002:
        return std::format("{}WM_DESTROY", prefix);
    case 0x0003:
        return std::format("{}WM_MOVE({:d}, {:d})", prefix,
            LOWORD(lParam), HIWORD(lParam));
    case 0x0005:
        return std::format("{}WM_SIZE({:d}, ({:d} x {:d}))", prefix,
            wParam, LOWORD(lParam), HIWORD(lParam));
    case 0x0006:
        return std::format("{}WM_ACTIVATE([{:d}|{:s}], [{}])", prefix,
            LOWORD(wParam), static_cast<bool>(HIWORD(wParam)),
            toString(reinterpret_cast<HWND>(lParam)));
    case 0x0007:
        return std::format("{}WM_SETFOCUS([{}])", prefix,
            toString(reinterpret_cast<HWND>(wParam)));
    case 0x0008:
        return std::format("{}WM_KILLFOCUS([{}])", prefix,
            toString(reinterpret_cast<HWND>(wParam)));
    case 0x000f:
        return std::format("{}WM_PAINT", prefix);
    case 0x0010:
        return std::format("{}WM_CLOSE", prefix);
    case 0x0014:
        return std::format("{}WM_ERASEBKGND({:#x})", prefix, wParam);
    case 0x0018:
        return std::format("{}WM_SHOWWINDOW({:s}, {:d})", prefix,
            static_cast<bool>(wParam), lParam);
    case 0x001c:
        return std::format("{}WM_ACTIVATEAPP({:s}, {:#x})", prefix,
            static_cast<bool>(wParam), lParam);
    case 0x0020:
        return std::format("{}WM_SETCURSOR([{}], [{:d}|{:#04x}])", prefix,
            toString(reinterpret_cast<HWND>(wParam)),
            LOWORD(lParam), HIWORD(lParam));
    case 0x0024:
        return std::format("{}WM_GETMINMAXINFO", prefix);
    case 0x0046:
        return std::format("{}WM_WINDOWPOSCHANGING({})", prefix,
            formatAsWINDOWPOS(lParam));
    case 0x0047:
        return std::format("{}WM_WINDOWPOSCHANGED", prefix,
            formatAsWINDOWPOS(lParam));
    case 0x007f:
        return std::format("{}WM_GETICON({:d})", prefix, wParam);
    case 0x0081:
        return std::format("{}WM_NCCREATE(...)", prefix);
    case 0x0082:
        return std::format("{}WM_NCDESTROY", prefix);
    case 0x0083:
        return std::format("{}WM_NCCALCSIZE({:s}, ...)", prefix,
            static_cast<bool>(wParam));
    case 0x0084:
        return std::format("{}WM_NCHITTEST({})", prefix,
            formatAsPOINTS(lParam));
    case 0x0085:
        return std::format("{}WM_NCPAINT(...)", prefix);
    case 0x0086:
        return std::format("{}WM_NCACTIVATE({:s}, {})", prefix,
            static_cast<bool>(wParam),
            toString(reinterpret_cast<HWND>(lParam)));
    case 0x00a0:
        return std::format("{}WM_NCMOUSEMOVE({:d}, {})", prefix,
            wParam, formatAsPOINTS(lParam));
    case 0x011f:
        return std::format("{}WM_MENUSELECT(...)", prefix);
    case 0x0121:
        return std::format("{}WM_ENTERIDLE(...)", prefix);
    case 0x0200:
        return std::format("{}WM_MOUSEMOVE({:#x}, {})", prefix,
            wParam, formatAsPOINTS(lParam));
    case 0x0214:
        return std::format("{}WM_SIZING({:d}, {})", prefix,
            wParam, formatAsRECT(lParam));
    case 0x0216:
        return std::format("{}WM_MOVING({})", prefix,
            formatAsRECT(lParam));
    case 0x02a2:
        return std::format("{}WM_NCMOUSELEAVE", prefix);
    default: return std::format(
        "{}msg({:#06x})({:#018x}, {:#018x})",
        prefix, msg, wParam, lParam);
    }
}


void logWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static std::set<UINT> mouseMessages = {
        WM_ENTERIDLE,
        WM_SETCURSOR,
        WM_MOUSEMOVE,
        WM_NCHITTEST,
        WM_NCMOUSEMOVE,
        WM_NCMOUSELEAVE,
    };
    if (mouseMessages.contains(msg)) {
        WITH_LOGGER_PREFIX("WinAPI.WndProc.MouseMove", "WndProc: ");
        LOG_TRACE("{}", formatMessage(hWnd, msg, wParam, lParam));
    } else {
        WITH_LOGGER_PREFIX("WinAPI.WndProc", "WndProc: ");
        LOG_TRACE("{}", formatMessage(hWnd, msg, wParam, lParam));
    }
}

}  // namespace


std::string toString(LPCWSTR w) noexcept {
    if (!w) {
        return std::string();
    }
    int bufferSize = WideCharToMultiByte(
        CP_UTF8, NULL, w, -1, nullptr, 0, NULL, NULL);
    if (bufferSize == 0) {
        return std::string();
    }
    std::string result(bufferSize - 1, '\0');
    WideCharToMultiByte(
        CP_UTF8, NULL, w, -1, result.data(), bufferSize, NULL, NULL);
    return result;
}


std::string formatMessage(const WindowMessage& message) noexcept {
    return formatMessage(
        message.hWnd, message.msg, message.wParam, message.lParam);
}


LRESULT handleMessageByDefault(const WindowMessage& message) noexcept {
    return DefWindowProc(
        message.hWnd, message.msg, message.wParam, message.lParam);
}


LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    logWindowProc(hWnd, msg, wParam, lParam);
    if (auto* handler = reinterpret_cast<WinAPI::WindowMessageHandler*>(
            GetWindowLongPtr(hWnd, GWLP_USERDATA))
    ) {
        return (*handler)(WinAPI::WindowMessage(hWnd, msg, wParam, lParam));
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

}  // namespace Istok::WinAPI
