#include <ecs.hpp>

#include <windows.h>
#include <iostream>
#include <unordered_set>
#include <functional>
#include <initializer_list>

using namespace Istok::ECS;

class ECSManager;

using System = std::function<void(ECSManager&)>;

class ECSManager {
public:
    ECSManager(std::initializer_list<System> args)
    : ecm(std::make_unique<EntityComponentManager>()), systems(args) {}

    ECSManager(const ECSManager&) = delete;
    ECSManager& operator=(const ECSManager&) = delete;
    ECSManager(ECSManager&&) = default;
    ECSManager& operator=(ECSManager&&) = default;

    ~ECSManager() {
        // Systems and components may have references to ECS
        // They must be destroyed first
        systems.clear();
        ecm.reset();
    }

    void addSystem(System system) {
        systems.push_back(system);
    }

    void iterate() {
        std::cout << "ECS iteration" << std::endl;
        for (auto& s : systems) {
            s(*this);
        }
    }

    void run() {
        if (running) {
            return;
        }
        running = true;
        while (running) {
            iterate();
        }
    }

    void stop() {
        running = false;
    }

    bool isValidEntity(Entity e) const {
        return ecm->isValidEntity(e);
    }

    template <typename Component>
    bool has(Entity e) const {
        assert(isValidEntity(e));
        return ecm->has<Component>(e);
    }

    template <typename Component>
    Component& get(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return ecm->get<Component>(e);
    }

    template <typename Component>
    const Component& get(Entity e) const {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return ecm->get<Component>(e);
    }

    template<typename... Components>
    Entity createEntity(Components&&... components) {
        Entity e = ecm->createEntity();
        (ecm->set(e, std::forward<Components>(components)), ...);
        return e;
    }

    void destroyEntity(Entity e) {
        assert(isValidEntity(e));
        ecm->destroyEntity(e);
    }

    template <typename Component>
    void set(Entity e, Component&& component) {
        assert(isValidEntity(e));
        ecm->set(e, std::move(component));
    }

    template <typename Component>
    void remove(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        ecm->remove<Component>(e);
    }

    template <typename Component>
    void removeAll() {
        ecm->removeAll<Component>();
    }

    template<typename... Components>
    EntityView view() {
        return ecm->view<Components...>();
    }

private:
    bool running = false;
    std::unique_ptr<EntityComponentManager> ecm;
    std::vector<System> systems;
};


template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};

HINSTANCE getHInstance() {
    static HINSTANCE hInstance =
        reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
    return hInstance;
}

class WindowClass {
public:
    WindowClass() = default;

    WindowClass(WNDPROC lpfnWndProc, LPCWSTR className)
    : name(className) {
        WNDCLASSEX wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_OWNDC;
        wcex.lpfnWndProc = lpfnWndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = getHInstance();
        wcex.hIcon = nullptr;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = className;
        wcex.hIconSm = nullptr;
        if (!RegisterClassEx(&wcex)) {
            throw std::runtime_error("Failed to register window class.");
        }
    }

    ~WindowClass() noexcept {
        UnregisterClass(name, getHInstance());
    }

    WindowClass(const WindowClass&) = delete;
    WindowClass& operator=(const WindowClass&) = delete;
    WindowClass(WindowClass&& other) = delete;
    WindowClass& operator=(WindowClass&& other) = delete;

    LPCWSTR get() const {
        return name;
    }

private:
    LPCWSTR name = nullptr;
};

struct ScreenLocation {
    Rect<int> value;
};

namespace WindowHandler {

struct Close {
    std::function<void()> func;
};

} // namespace WindowHandler

struct SysWindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};


LRESULT handleByDefault(SysWindowMessage message) {
    return DefWindowProc(
        message.hWnd,
        message.msg,
        message.wParam,
        message.lParam);
}


struct ECSBinding {
    ECSManager* ecs;
    Entity entity;
};


class WindowMessageHandler {
public:
    virtual ~WindowMessageHandler() = default;
    virtual LRESULT handleWindowMessage(
        SysWindowMessage message, ECSBinding binding) noexcept = 0;
};


class Window {
private:
    class HandlerProxy {
    public:
        HandlerProxy(WindowMessageHandler& handler, ECSBinding binding)
        : handler(handler), binding(binding) {}

        LRESULT handle(SysWindowMessage message) noexcept {
            return handler.handleWindowMessage(message, binding);
        }

    private:
        WindowMessageHandler& handler;
        ECSBinding binding;
    };

public:
    Window(WindowMessageHandler& handler, ECSBinding binding)
    : handler(handler, binding) {
        Rect<int> location =
            binding.ecs->get<ScreenLocation>(binding.entity).value;
        hWnd = CreateWindowEx(
            NULL,
            getWindowClass(),
            L"Istok",
            WS_OVERLAPPEDWINDOW,
            location.left, location.top,
            location.right - location.left,
            location.bottom - location.top,
            NULL, NULL, getHInstance(), nullptr);
        if (!hWnd) {
            throw std::runtime_error("Cannot create window");
        }
        ShowWindow(hWnd, SW_SHOW);
        SetWindowLongPtr(
            hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&this->handler));
        std::cout << "Window created: " << hWnd << std::endl;
    }
    
    ~Window() {
        if (hWnd) {
            std::cout << "Destroying window: " << hWnd << std::endl;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
            DestroyWindow(hWnd);
            std::cout << "Window destroyed: " << hWnd << std::endl;
        }
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& source) = delete;
    Window& operator=(Window&& source) = delete;

    HWND getHWnd() const {
        return hWnd;
    }

private:
    HWND hWnd = nullptr;
    HandlerProxy handler;

    static LPCWSTR getWindowClass() {
        static WindowClass wc(windowProc, L"Istok");
        return wc.get();
    }

    static LRESULT CALLBACK windowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
    ) noexcept {
        if (HandlerProxy* handler = reinterpret_cast<HandlerProxy*>(
            GetWindowLongPtr(hWnd, GWLP_USERDATA))
        ) {
            return handler->handle(SysWindowMessage(hWnd, msg, wParam, lParam));
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
};

struct WindowState {
    bool idle;
};


class CloseHandler : public WindowMessageHandler {
public:
    CloseHandler() = default;
    
    LRESULT handleWindowMessage(
        SysWindowMessage message, ECSBinding binding
    ) noexcept override {
        assert(message.msg == WM_CLOSE);
        ECSManager& ecs = *binding.ecs;
        Entity entity = binding.entity;
        auto stateView = ecs.view<WindowState>();
        assert(stateView.begin() != stateView.end());
        auto global = *stateView.begin();
        ecs.set(global, WindowState{false});
        if (ecs.has<WindowHandler::Close>(entity)) {
            std::cout << "handler: WM_CLOSE " <<
                entity.value << std::endl;
            ecs.get<WindowHandler::Close>(entity).func();
            return 0;
        }
        return handleByDefault(message);
    }
};


class SizeHandler : public WindowMessageHandler {
public:
    SizeHandler() = default;
    
    LRESULT handleWindowMessage(
        SysWindowMessage message, ECSBinding binding
    ) noexcept override {
        assert(message.msg == WM_SIZE);
        ECSManager& ecs = *binding.ecs;
        Entity entity = binding.entity;
        auto stateView = ecs.view<WindowState>();
        assert(stateView.begin() != stateView.end());
        auto global = *stateView.begin();
        ecs.set(global, WindowState{false});
        if (ecs.has<std::unique_ptr<Window>>(entity)) {
            std::cout << "message: WM_SIZE " <<
                entity.value << std::endl;
            ecs.iterate();
        }
        return 0;
    }

private:
    std::unordered_map<UINT, std::unique_ptr<WindowMessageHandler>> handlers;
};


class Handler : public WindowMessageHandler {
public:
    Handler() {
        handlers[WM_CLOSE] = std::make_unique<CloseHandler>();
        handlers[WM_SIZE] = std::make_unique<SizeHandler>();
    }
    
    LRESULT handleWindowMessage(
        SysWindowMessage message, ECSBinding binding
    ) noexcept override {
        ECSManager& ecs = *binding.ecs;
        auto stateView = ecs.view<WindowState>();
        if (
            stateView.begin() != stateView.end() &&
            handlers.contains(message.msg)
        ) {
            return handlers[message.msg]->handleWindowMessage(message, binding);
        }
        return handleByDefault(message);
    }

private:
    std::unordered_map<UINT, std::unique_ptr<WindowMessageHandler>> handlers;
};


void createWindows(ECSManager& ecs) {
    static Handler handler;
    for (auto& w : ecs.view<ScreenLocation>()
            .exclude<std::unique_ptr<Window>>()
    ) {
        std::cout << "Creating window for entity " << w.value << std::endl;
        Rect<int>& location = ecs.get<ScreenLocation>(w).value;
        ecs.set(w, std::make_unique<Window>(handler, ECSBinding{&ecs, w}));
    }
}


void processWindowsMessages(ECSManager& ecs) {
    auto stateView = ecs.view<WindowState>();
    auto global = (stateView.begin() == stateView.end())
        ? ecs.createEntity(WindowState{true})
        : *stateView.begin();
    while (ecs.get<WindowState>(global).idle) {
        MSG msg;
        GetMessage(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            ecs.stop();
            return;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ecs.set(global, WindowState{true});
}


int main() {
    ECSManager ecs {
        System{createWindows},
        System{processWindowsMessages},
    };
    ecs.createEntity(
        ScreenLocation{{200, 100, 600, 400}},
        WindowHandler::Close{[&](){
            ecs.createEntity(
                ScreenLocation{{300, 200, 500, 500}},
                WindowHandler::Close{[&](){ ecs.stop(); }});
        }});
    ecs.run();
    return 0;
}
