// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "definitions.hpp"
#include <tools/queue.hpp>
#include <tools/helpers.hpp>
#include <windows.h>
#include <memory>
#include <unordered_map>

namespace Istok::GUI::WinAPI {

class MessageHandler {};


template <typename ID>
class Window {
public:
    Window(WindowParams params, MessageHandler& handler) {}
    
    HWND getHWnd() {
        return 0;
    }

    void loadScene(std::unique_ptr<Scene<ID>>&& scene) {
        this->scene = *scene;
    }

private:
    Scene<ID> scene;
};


template <typename ID_>
class Platform: public MessageHandler {
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

    void createWindow(ID id, WindowParams params) {
        auto window = std::make_unique<Window<ID>>(params, *this);
        HWND hWnd = window->getHWnd();
        windows[hWnd] = std::move(window);
        idHWndMap[id] = hWnd;
        ShowWindow(hWnd, SW_SHOW);
    }

    void destroyWindow(ID id) {
        HWND hWnd = idHWndMap[id];
        idHWndMap.erase(id);
        windows.erase(hWnd);
    }

    void loadScene(ID windowID, std::unique_ptr<Scene<ID>>&& scene) {
        HWND hWnd = idHWndMap[windowID];
        Window<ID>& window = *windows[hWnd];
        window.loadScene(std::move(scene));
    }
    
private:
    Tools::SimpleQueue<PlatformEvent<ID>> outQueue;
    std::unordered_map<HWND, std::unique_ptr<Window<ID>>> windows;
    std::unordered_map<ID, HWND, Tools::hash<ID>> idHWndMap;
};

}
