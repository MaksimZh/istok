// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "message.hpp"

#include <format>
#include <string>
#include <windows.h>
#include <windowsx.h>


namespace Istok::GUI::WinAPI {

namespace {

std::string toString(LPCWSTR w) {
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

std::string formatAsCREATESTRUCT(LPARAM lParam) {
    auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
    return std::format(
        "CREATESTRUCT(({:d}, {:d}), ({:d} x {:d}), {})",
        cs->x, cs->y, cs->cx, cs->cy, toString(cs->lpszName));
}

std::string formatAsWINDOWPOS(LPARAM lParam) {
    auto wp = reinterpret_cast<WINDOWPOS*>(lParam);
    return std::format(
        "WINDOWPOS({}, ({:d}, {:d}), ({:d} x {:d}), {:#x})",
        wp->hwndInsertAfter, wp->x, wp->y, wp->cx, wp->cy, wp->flags);
}

std::string formatAsPOINTS(LPARAM lParam) {
    return std::format(
        "({:d}, {:d})",
        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
}

std::string formatAsRECT(LPARAM lParam) {
    auto rect = reinterpret_cast<RECT*>(lParam);
    return std::format(
        "RECT({:d}, {:d}, {:d}, {:d})",
        rect->left, rect->top, rect->right, rect->bottom);
}

}  // namespace


std::string toString(const WindowMessage& message) {
    switch (message.msg) {
    case 0x0001:
        return std::format("{}.WM_CREATE({})", message.hWnd,
            formatAsCREATESTRUCT(message.lParam));
    case 0x0002:
        return std::format("{}.WM_DESTROY", message.hWnd);
    case 0x0003:
        return std::format("{}.WM_MOVE{}", message.hWnd,
            formatAsPOINTS(message.lParam));
    case 0x0005:
        return std::format("{}.WM_SIZE({:d}, {})", message.hWnd,
            message.wParam, formatAsPOINTS(message.lParam));
    case 0x0006:
        return std::format("{}.WM_ACTIVATE([{:d}|{:s}], [{}])", message.hWnd,
            LOWORD(message.wParam), static_cast<bool>(HIWORD(message.wParam)),
            reinterpret_cast<HWND>(message.lParam));
    case 0x0007:
        return std::format("{}.WM_SETFOCUS([{}])", message.hWnd,
            reinterpret_cast<HWND>(message.wParam));
    case 0x0008:
        return std::format("{}.WM_KILLFOCUS([{}])", message.hWnd,
            reinterpret_cast<HWND>(message.wParam));
    case 0x000f:
        return std::format("{}.WM_PAINT", message.hWnd);
    case 0x0010:
        return std::format("{}.WM_CLOSE", message.hWnd);
    case 0x0014:
        return std::format("{}.WM_ERASEBKGND({:#x})", message.hWnd,
            message.wParam);
    case 0x0018:
        return std::format("{}.WM_SHOWWINDOW({:s}, {:d})", message.hWnd,
            static_cast<bool>(message.wParam), message.lParam);
    case 0x001c:
        return std::format("{}.WM_ACTIVATEAPP({:s}, {:#x})", message.hWnd,
            static_cast<bool>(message.wParam), message.lParam);
    case 0x0020:
        return std::format(
            "{}.WM_SETCURSOR([{}], [{:d}|{:#04x}])", message.hWnd,
            reinterpret_cast<HWND>(message.wParam),
            LOWORD(message.lParam), HIWORD(message.lParam));
    case 0x0024:
        return std::format("{}.WM_GETMINMAXINFO", message.hWnd);
    case 0x0046:
        return std::format("{}.WM_WINDOWPOSCHANGING({})", message.hWnd,
            formatAsWINDOWPOS(message.lParam));
    case 0x0047:
        return std::format("{}.WM_WINDOWPOSCHANGED", message.hWnd,
            formatAsWINDOWPOS(message.lParam));
    case 0x007f:
        return std::format("{}.WM_GETICON({:d})", message.hWnd, message.wParam);
    case 0x0081:
        return std::format("{}.WM_NCCREATE(...)", message.hWnd);
    case 0x0082:
        return std::format("{}.WM_NCDESTROY", message.hWnd);
    case 0x0083:
        return std::format("{}.WM_NCCALCSIZE({:s}, ...)", message.hWnd,
            static_cast<bool>(message.wParam));
    case 0x0084:
        return std::format("{}.WM_NCHITTEST({})", message.hWnd,
            formatAsPOINTS(message.lParam));
    case 0x0085:
        return std::format("{}.WM_NCPAINT(...)", message.hWnd);
    case 0x0086:
        return std::format("{}.WM_NCACTIVATE({:s}, {})", message.hWnd,
            static_cast<bool>(message.wParam),
            reinterpret_cast<HWND>(message.lParam));
    case 0x00a0:
        return std::format("{}.WM_NCMOUSEMOVE({:d}, {})", message.hWnd,
            message.wParam, formatAsPOINTS(message.lParam));
    case 0x011f:
        return std::format("{}.WM_MENUSELECT(...)", message.hWnd);
    case 0x0121:
        return std::format("{}.WM_ENTERIDLE(...)", message.hWnd);
    case 0x0200:
        return std::format("{}.WM_MOUSEMOVE({:#x}, {})", message.hWnd,
            message.wParam, formatAsPOINTS(message.lParam));
    case 0x0214:
        return std::format("{}.WM_SIZING({:d}, {})", message.hWnd,
            message.wParam, formatAsRECT(message.lParam));
    case 0x0216:
        return std::format("{}.WM_MOVING({})", message.hWnd,
            formatAsRECT(message.lParam));
    case 0x02a2:
        return std::format("{}.WM_NCMOUSELEAVE", message.hWnd);
    default: return std::format(
        "{}.msg({:#06x})({:#018x}, {:#018x})",
        message.hWnd, message.msg, message.wParam, message.lParam);
    }
}

}  // namespace Istok::GUI::WinAPI
