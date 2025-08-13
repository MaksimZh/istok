// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <string>

namespace {

class FakePlatform {
    class Notifier {};
};

}

TEST_CASE("GUI - Core", "[unit][gui]") {
    using AppQueue = SyncWaitingQueue<std::string>;
    FakePlatform platform;
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Core<FakePlatform, AppQueue> core(platform, appQueue);
}
