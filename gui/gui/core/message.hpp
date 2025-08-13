// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <variant>

namespace Istok::GUI {

namespace Message {

struct GUIExit {};

}

using GUIMessage = std::variant<Message::GUIExit>;

class AppMessage {};

class WindowMessageHandler {};

} // namespace Istok::GUI
