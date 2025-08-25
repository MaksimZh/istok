// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "definitions.hpp"
#include <tools/queue.hpp>
#include <windows.h>
#include <memory>

namespace Istok::GUI::WinAPI {

template <typename ID_>
class Platform {
public:
    using ID = ID_;

    Platform() {}

    PlatformEvent<ID> getMessage() noexcept {
        while (true) {
            if (!outQueue.empty()) {
                return outQueue.take();
            }
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                return Event::PlatformShutdown{};
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void createWindow(ID id, WindowParams params) {}
    
    void destroyWindow(ID id) {}

    void setWindowLocation(ID id, Rect<int> location) {}
    
    void loadScene(ID window, std::unique_ptr<Scene<ID>>&& scene) {}
    
    void patchScene(ID window, std::unique_ptr<ScenePatch<ID>>&& patch) {}

private:
    Tools::SimpleQueue<PlatformEvent<ID>> outQueue;
};


}
