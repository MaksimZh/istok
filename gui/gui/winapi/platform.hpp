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
    PlatformCommands::SetAreaTester<ID>,
    PlatformEvents::WindowClose<ID>,
    PlatformMessages::WindowCreated<ID>,
    PlatformMessages::WindowDestroyed<ID>,
    PlatformMessages::AddWindowHandler<ID>
>;


template <typename ID>
class WindowCloseMessageHandler {
public:
    WindowCloseMessageHandler(ID id, Tools::Sink<PlatformEvent<ID>>& output)
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
    Tools::Sink<PlatformEvent<ID>>& output;
};


template <typename ID>
class WindowCloseHandler {
public:
    WindowCloseHandler(
        Tools::Sink<PlatformMessage<ID>>& bus,
        Tools::Sink<PlatformEvent<ID>>& output)
    : bus(bus), output(output) {}

    Tools::HandlerResult<PlatformMessage<ID>> operator()(
        PlatformMessage<ID>&& message
    ) {
        return std::visit(
            [this](auto&& x) { return this->handle(std::move(x)); },
            std::move(message));
    }

private:
    Tools::Sink<PlatformMessage<ID>>& bus;
    Tools::Sink<PlatformEvent<ID>>& output;

    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformMessages::WindowCreated<ID>&& message
    ) {
        bus.push(PlatformMessages::AddWindowHandler(
            message.id,
            Tools::makeSharedHandler<
                WindowCloseMessageHandler<ID>, WindowMessage>(
                    message.id, output)));
        return Tools::HandlerResult<PlatformMessage<ID>>(std::move(message));
    }

    template <typename T>
    Tools::HandlerResult<PlatformMessage<ID>> handle(T&& message) {
        return Tools::HandlerResult<PlatformMessage<ID>>(std::move(message));
    }
};


class WindowAreaTestMessageHandler {
public:
    using Result = Tools::HandlerResult<WindowMessage, WindowResult>;
    
    Result operator()(
        WindowMessage&& message
    ) noexcept {
        if (message.msg != WM_NCHITTEST || !tester) {
            return Result::fromArgument(std::move(message));
        }
        RECT rect;
        GetWindowRect(message.hWnd, &rect);
        Position<int> position(
            GET_X_LPARAM(message.lParam) - rect.left,
            GET_Y_LPARAM(message.lParam) - rect.top);
        switch (tester->testWindowArea(position)) {
        case WindowArea::hole: return Result::fromResult(HTTRANSPARENT);
        case WindowArea::client: return Result::fromResult(HTCLIENT);
        case WindowArea::moving: return Result::fromResult(HTCAPTION);
        case WindowArea::sizingTL: return Result::fromResult(HTTOPLEFT);
        case WindowArea::sizingT: return Result::fromResult(HTTOP);
        case WindowArea::sizingTR: return Result::fromResult(HTTOPRIGHT);
        case WindowArea::sizingR: return Result::fromResult(HTRIGHT);
        case WindowArea::sizingBR: return Result::fromResult(HTBOTTOMRIGHT);
        case WindowArea::sizingB: return Result::fromResult(HTBOTTOM);
        case WindowArea::sizingBL: return Result::fromResult(HTBOTTOMLEFT);
        case WindowArea::sizingL: return Result::fromResult(HTLEFT);
        default: return Result::fromResult(HTCLIENT);
        }
    }

    void setAreaTester(std::unique_ptr<WindowAreaTester>&& tester) {
        this->tester = std::move(tester);
    }

private:
    std::unique_ptr<WindowAreaTester> tester;
};


template <typename ID>
class WindowAreaTestHandler {
public:
    WindowAreaTestHandler(Tools::Sink<PlatformMessage<ID>>& bus)
    : bus(bus) {}

    Tools::HandlerResult<PlatformMessage<ID>> operator()(
        PlatformMessage<ID>&& message
    ) {
        return std::visit(
            [this](auto&& x) { return this->handle(std::move(x)); },
            std::move(message));
    }

private:
    Tools::Sink<PlatformMessage<ID>>& bus;
    std::unordered_map<
        ID,
        std::shared_ptr<WindowAreaTestMessageHandler>,
        Tools::hash<ID>
    > handlers;

    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformMessages::WindowCreated<ID>&& message
    ) {
        handlers[message.id] =
            std::make_shared<WindowAreaTestMessageHandler>();
        bus.push(PlatformMessages::AddWindowHandler(
            message.id,
            Tools::wrapSharedHandler<WindowMessage>(handlers[message.id])));
        return Tools::HandlerResult<PlatformMessage<ID>>(std::move(message));
    }

    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformMessages::WindowDestroyed<ID>&& message
    ) {
        handlers.erase(message.id);
        return Tools::HandlerResult<PlatformMessage<ID>>(std::move(message));
    }

    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformCommands::SetAreaTester<ID>&& message
    ) {
        handlers.at(message.id)->setAreaTester(std::move(message.tester));
        return Tools::HandlerResult<PlatformMessage<ID>>();
    }

    template <typename T>
    Tools::HandlerResult<PlatformMessage<ID>> handle(T&& message) {
        return Tools::HandlerResult<PlatformMessage<ID>>(std::move(message));
    }
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
        Tools::Sink<PlatformMessage<ID>>& bus
    ) : windows(windows), bus(bus) {}

    Tools::HandlerResult<PlatformMessage<ID>> operator()(
        PlatformMessage<ID>&& message
    ) {
        return std::visit(
            [this](auto&& x) { return this->handle(std::move(x)); },
            std::move(message));
    }

private:
    WindowManager<ID, Window>& windows;
    Tools::Sink<PlatformMessage<ID>>& bus;
    
    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformCommands::CreateWindow<ID>&& message
    ) {
        windows.create(message.id, message.params);
        bus.push(PlatformMessages::WindowCreated<ID>(message.id));
        return Tools::HandlerResult<PlatformMessage<ID>>();
    }

    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformCommands::DestroyWindow<ID>&& message
    ) {
        windows.destroy(message.id);
        bus.push(PlatformMessages::WindowDestroyed<ID>(message.id));
        return Tools::HandlerResult<PlatformMessage<ID>>();
    }

    Tools::HandlerResult<PlatformMessage<ID>> handle(
        PlatformMessages::AddWindowHandler<ID>&& message
    ) {
        windows.getWindow(message.id).appendHandler(message.handler);
        return Tools::HandlerResult<PlatformMessage<ID>>();
    }

    template <typename T>
    Tools::HandlerResult<PlatformMessage<ID>> handle(T&& message) {
        return Tools::HandlerResult<PlatformMessage<ID>>(std::move(message));
    }
};


template <typename ID_, GUIWindow Window>
class Platform: public EventHandler<Window> {
public:
    using ID = ID_;
    using Scene = Window::Scene;

    Platform(std::unique_ptr<WindowFactoryBuilder<Window>> windowFactoryBuilder)
    : windows(std::move(buildWindowFactory(std::move(windowFactoryBuilder)))) {
        bus.addSubscriber(Tools::makeSharedHandler<
            WindowHandler<ID, Window>, PlatformMessage<ID>>(windows, bus));
        bus.addSubscriber(Tools::makeSharedHandler<
            WindowCloseHandler<ID>, PlatformMessage<ID>>(bus, outQueue));
        bus.addSubscriber(Tools::makeSharedHandler<
            WindowAreaTestHandler<ID>, PlatformMessage<ID>>(bus));
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
