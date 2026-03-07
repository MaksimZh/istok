// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window.hpp"

#include <cassert>
#include <memory>
#include <windows.h>

#include "delegate.hpp"
#include "message.hpp"


namespace Istok::GUI::WinAPI {

Window::Window(WinAPIDelegate& winapi, Rect<int> location) {
    HWND hWnd = winapi.createWindow(location);
    if (!hWnd) {
        LOG_ERROR("Window creation failed.");
        return;
    }
    LOG_DEBUG("Created window: {}", hWnd);
    winapi_ = &winapi;
    hWnd_ = hWnd;
}

Window::~Window() {
    clear();
}

Window::Window(Window&& source) {
    takeFrom(source);
}

Window& Window::operator=(Window&& source) {
    if (&source != this) {
        clear();
        takeFrom(source);
    }
    return *this;
}

void Window::setMessageHandler(WindowMessageHandler&& handler) {
    if (!hWnd_) {
        return;
    }
    handler_ = std::make_unique<WindowMessageHandler>(std::move(handler));
    assert(winapi_);
    winapi_->setUserPointer(hWnd_, handler_.get());
}

void Window::resetMessageHandler() {
    if (!hWnd_ || !handler_) {
        return;
    }
    assert(winapi_);
    winapi_->setRawUserPointer(hWnd_, NULL);
    handler_.reset();
}

void Window::takeFrom(Window& source) {
    winapi_ = source.winapi_;
    hWnd_ = source.hWnd_;
    handler_ = std::move(source.handler_);
    source.winapi_ = nullptr;
    source.hWnd_ = nullptr;
}

void Window::clear() {
    if (!hWnd_) {
        return;
    }
    LOG_DEBUG("Destroying window {}", hWnd_);
    SetWindowLongPtr(hWnd_, GWLP_USERDATA, NULL);
    DestroyWindow(hWnd_);
    winapi_ = nullptr;
    hWnd_ = nullptr;
}

}  // namespace Istok::GUI::WinAPI
