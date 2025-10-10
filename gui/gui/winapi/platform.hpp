// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <gui/common/platform.hpp>
#include <tools/exchange.hpp>
#include <tools/helpers.hpp>

#include <windows.h>
#include <windowsx.h>
#undef CreateWindow
#undef DestroyWindow

#include <stdexcept>
#include <memory>
#include <unordered_map>

namespace Istok::GUI::WinAPI {

template <typename Window>
class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void onException(std::exception_ptr exception) noexcept = 0;
};


template <typename ID, typename Window>
class WindowMap {
public:
    void insert(ID id, std::unique_ptr<Window>&& window) {
        if (identifiers.contains(window.get()) || windows.contains(id)) {
            throw std::runtime_error("Window overwrite");
        }
        identifiers[window.get()] = id;
        windows[id] = std::move(window);
    }

    void erase(ID id) {
        if (!windows.contains(id)) {
            throw std::runtime_error("Window not found");
        }
        Window* window = windows[id].get();
        windows.erase(id);
        assert(identifiers.contains(window));
        identifiers.erase(window);
    }

    ID getID(Window* window) {
        if (!identifiers.contains(window)) {
            throw std::runtime_error("Window not found");
        }
        return identifiers[window];
    }

    Window& getWindow(ID id) {
        if (!windows.contains(id)) {
            throw std::runtime_error("Window not found");
        }
        return *windows[id];
    }

private:
    std::unordered_map<ID, std::unique_ptr<Window>, Tools::hash<ID>> windows;
    std::unordered_map<Window*, ID> identifiers;
};


template <typename Window>
class WindowFactory {
public:
    virtual ~WindowFactory() = default;
    virtual std::unique_ptr<Window> create(const WindowParams& params) = 0;
};


template <typename ID, typename Window>
class WindowManager {
public:
    WindowManager(std::unique_ptr<WindowFactory<Window>>&& factory)
    : factory(std::move(factory)) {}

    void create(ID id, const WindowParams& params) {
        windows.insert(id, factory->create(params));
    }

    void destroy(ID id) {
        windows.erase(id);
    }

    ID getID(Window* window) {
        return windows.getID(window);
    }

    Window& getWindow(ID id) {
        return windows.getWindow(id);
    }

private:
    std::unique_ptr<WindowFactory<Window>> factory;
    WindowMap<ID, Window> windows;
};


template <typename Window>
class WindowFactoryBuilder {
public:
    virtual ~WindowFactoryBuilder() = default;
    virtual std::unique_ptr<WindowFactory<Window>> buildWindowFactory(
        EventHandler<Window>& eventHandler) = 0;
};


template <typename Window>
concept GUIWindow = requires {
    typename Window::Scene;
} && requires(
    Window window,
    std::unique_ptr<typename Window::Scene>&& scene,
    std::unique_ptr<WindowAreaTester>&& areaTester
) {
    {window.loadScene(std::move(scene))} -> std::same_as<void>;
    {window.setAreaTester(std::move(areaTester))} -> std::same_as<void>;
};


struct WindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

using WindowResult = LRESULT;


namespace PlatformMessages {
    template <typename ID>
    struct WindowCreated {
        ID id;
    };

    template <typename ID>
    struct WindowDestroyed {
        ID id;
    };

    template <typename ID>
    struct AddWindowHandler {
        ID id;
        Tools::Handler<WindowMessage, WindowResult> handler;
    };
}

template <typename ID>
using PlatformMessage = std::variant<
    PlatformCommands::HeartbeatRequest,
    PlatformCommands::CreateWindow<ID>,
    PlatformCommands::DestroyWindow<ID>,
    PlatformEvents::WindowClose<ID>,
    PlatformMessages::WindowCreated<ID>,
    PlatformMessages::WindowDestroyed<ID>,
    PlatformMessages::AddWindowHandler<ID>
>;


template <typename ID>
class WindowCloseHandler {
public:
    WindowCloseHandler(ID id, Tools::Sink<PlatformMessage<ID>>& output)
    : id(id), output(output) {}

    Tools::HandlerResult<WindowMessage, WindowResult> operator()(
        WindowMessage&& message
    ) noexcept {
        if (message.msg != WM_CLOSE) {
            return Tools::HandlerResult<WindowMessage, WindowResult>::
                fromArgument(std::move(message));
        }
        output.push(PlatformEvents::WindowClose(id));
        return Tools::HandlerResult<WindowMessage, WindowResult>::
                fromResult(0);
    }

private:
    ID id;
    Tools::Sink<PlatformMessage<ID>>& output;
};


class WindowAreaTestHandler {
public:
    std::optional<LRESULT> operator()(WindowMessage message) noexcept {
        if (!tester) {
            return HTCLIENT;
        }
        RECT rect;
        GetWindowRect(message.hWnd, &rect);
        Position<int> position(
            GET_X_LPARAM(message.lParam) - rect.left,
            GET_Y_LPARAM(message.lParam) - rect.top);
        switch (tester->testWindowArea(position)) {
        case WindowArea::hole: return HTTRANSPARENT;
        case WindowArea::client: return HTCLIENT;
        case WindowArea::moving: return HTCAPTION;
        case WindowArea::sizingTL: return HTTOPLEFT;
        case WindowArea::sizingT: return HTTOP;
        case WindowArea::sizingTR: return HTTOPRIGHT;
        case WindowArea::sizingR: return HTRIGHT;
        case WindowArea::sizingBR: return HTBOTTOMRIGHT;
        case WindowArea::sizingB: return HTBOTTOM;
        case WindowArea::sizingBL: return HTBOTTOMLEFT;
        case WindowArea::sizingL: return HTLEFT;
        default: return HTCLIENT;
        }
    }

    void setAreaTester(std::unique_ptr<WindowAreaTester>&& tester) {
        this->tester = std::move(tester);
    }

private:
    std::unique_ptr<WindowAreaTester> tester;
};



template <typename ID>
class PlatformCommandHandler {
public:
    virtual ~PlatformCommandHandler() = default;
    virtual void handlePlatformCommand(PlatformMessage<ID> command) = 0;
};


template <typename ID, typename Window>
class WindowHandler {
public:
    WindowHandler(
        WindowManager<ID, Window>& windows,
        Tools::Sink<PlatformMessage<ID>>& output
    ) : windows(windows), output(output) {}

    Tools::HandlerResult<PlatformMessage<ID>> operator()(
        PlatformMessage<ID>&& command
    ) {
        return std::visit(
            [this](auto&& x) { return this->handle(std::move(x)); },
            std::move(command));
    }

private:
    WindowManager<ID, Window>& windows;
    Tools::Sink<PlatformMessage<ID>>& output;
    std::vector<std::unique_ptr<WindowCloseHandler<ID>>> handlers;
    
    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformCommands::CreateWindow<ID>&& command
    ) {
        windows.create(command.id, command.params);
        handlers.push_back(std::make_unique<WindowCloseHandler<ID>>(
                command.id, output));
        windows.getWindow(command.id).appendHandler(*handlers.back());
        return Tools::HandlerResult<PlatformMessage<ID>>();
    }

    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformCommands::DestroyWindow<ID>&& command
    ) {
        windows.destroy(command.id);
        return Tools::HandlerResult<PlatformMessage<ID>>();
    }

    template <typename T>
    Tools::HandlerResult<PlatformMessage<ID>> handle(T&& command) {
        return Tools::HandlerResult<PlatformMessage<ID>>(std::move(command));
    }
};


template <typename ID>
class WindowCloseListener {
public:
    WindowCloseListener(Tools::Sink<PlatformEvent<ID>>& output)
    : output(output) {}

    Tools::HandlerResult<PlatformMessage<ID>> operator()(
        PlatformMessage<ID>&& command
    ) {
        return std::visit(
            [this](auto&& x) { return this->handle(std::move(x)); },
            std::move(command));
    }

private:
    Tools::Sink<PlatformEvent<ID>>& output;
    
    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformEvents::WindowClose<ID>&& command
    ) {
        output.push(std::move(command));
        return Tools::HandlerResult<PlatformMessage<ID>>();
    }

    template <typename T>
    Tools::HandlerResult<PlatformMessage<ID>> handle(T&& command) {
        return Tools::HandlerResult<PlatformMessage<ID>>(std::move(command));
    }
};


template <typename ID_, GUIWindow Window>
class Platform: public EventHandler<Window> {
public:
    using ID = ID_;
    using Scene = Window::Scene;

    Platform(std::unique_ptr<WindowFactoryBuilder<Window>> windowFactoryBuilder)
    : windows(std::move(buildWindowFactory(std::move(windowFactoryBuilder)))) {
        bus.addSubscriber(
            Tools::makeSharedHandler<
                WindowHandler<ID, Window>, PlatformMessage<ID>>(
                    windows, bus));
        bus.addSubscriber(
            Tools::makeSharedHandler<
                WindowCloseListener<ID>, PlatformMessage<ID>>(outQueue));
    }

    PlatformEvent<ID> getMessage() noexcept {
        while (true) {
            if (auto outMessage = outQueue.take()) {
                return outMessage.value();
            }
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                return PlatformEvents::Shutdown{};
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void sendCommand(PlatformCommand<ID>&& command) {
        try {
            bus.push(
                std::visit([](auto&& x) -> PlatformMessage<ID> {
                    return std::move(x);
                },
                command));
        } catch(...) {
            onException(std::current_exception());
        }
    }

    void setAreaTester(
        ID id, std::unique_ptr<WindowAreaTester>&& tester) noexcept
    {
        try {
            windows.getWindow(id).setAreaTester(std::move(tester));
        } catch(...) {
            onException(std::current_exception());
        }
    }

    void loadScene(
        ID id, std::unique_ptr<Scene>&& scene) noexcept
    {
        try {
            windows.getWindow(id).loadScene(std::move(scene));
        } catch(...) {
            onException(std::current_exception());
        }
    }

    void onException(std::exception_ptr exception) noexcept override {
        outQueue.push(PlatformEvents::Exception(exception));
    }
    
private:
    WindowManager<ID, Window> windows;
    Tools::Queue<PlatformEvent<ID>> outQueue;
    Tools::MessageBus<PlatformMessage<ID>> bus;

    std::unique_ptr<WindowFactory<Window>> buildWindowFactory(
        std::unique_ptr<WindowFactoryBuilder<Window>>&& windowFactoryBuilder
    ) {
        if (!windowFactoryBuilder) {
            throw std::runtime_error("No window factory builder found");
        }
        return windowFactoryBuilder->buildWindowFactory(*this);
    }
};

} // namespace Istok::GUI::WinAPI
