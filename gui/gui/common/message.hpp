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

template <typename ID>
struct GUINewWindow {
    ID id;
    WindowParams params;
};

template <typename ID>
struct GUIDestroyWindow {
    ID id;
};

template <typename ID>
struct AppWindowClosed {
    ID id;
};

}

template <typename ID>
using GUIMessage = std::variant<
    Message::GUIExit,
    Message::GUINewWindow<ID>,
    Message::GUIDestroyWindow<ID>
>;

template <typename ID>
using AppMessage = std::variant<
    Message::AppWindowClosed<ID>
>;

template <typename ID>
class WindowMessageHandler {
public:
    virtual void onMessage(GUIMessage<ID> msg) = 0;
    virtual void onClose(ID id) = 0;
};

} // namespace Istok::GUI
