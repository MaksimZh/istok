// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

namespace Istok::GUI {

class MessageHandler {};

class Notifier {};

class CoreChannel {
public:
    void setNotifier(Notifier&& notifier) {}
};

template <typename Platform>
class Core : MessageHandler {
public:
    Core(CoreChannel channel)
        : platform(*this), channel(channel) {
        this->channel.setNotifier(platform.getNotifier());
    }

private:
    Platform platform;
    CoreChannel channel;
};

} // namespace Istok::GUI
