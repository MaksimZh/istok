// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <optional>
#include <string>
#include <variant>

namespace Istok::GUI {

template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};

struct WindowParams {
    Rect<int> location;
    std::optional<std::string> title;
};

namespace Message {

struct GUIExit {};

template <typename WindowID>
struct GUINewWindow {
    WindowID id;
    WindowParams params;
};

template <typename WindowID>
struct GUIDestroyWindow {
    WindowID id;
};

struct AppGUIException {
    std::exception_ptr exception;
};

template <typename WindowID>
struct AppWindowClosed {
    WindowID id;
};

}

template <typename WindowID>
using GUIMessage = std::variant<
    Message::GUIExit,
    Message::GUINewWindow<WindowID>,
    Message::GUIDestroyWindow<WindowID>
>;

template <typename WindowID>
using AppMessage = std::variant<
    Message::AppGUIException,
    Message::AppWindowClosed<WindowID>
>;

template <typename WindowID>
using AppQueue = Tools::SyncWaitingQueue<AppMessage<WindowID>>;

template <typename WindowID>
using SharedAppQueue = std::shared_ptr<AppQueue<WindowID>>;


template <typename WindowID>
class GUIHandler {
public:
    virtual void onMessage(GUIMessage<WindowID> msg) noexcept = 0;
    virtual void onWindowClose(WindowID id) noexcept = 0;
};

} // namespace Istok::GUI
