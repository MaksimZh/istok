#include <ecs.hpp>
#include <gui/winapi/wgl.hpp>

#include <windows.h>
#include <iostream>
#include <unordered_set>
#include <functional>

using namespace Istok::ECS;

class System {
public:
    virtual ~System() = default;
    virtual void run() = 0;
};

class ECSManager {
public:
    ECSManager() {}

    ECSManager(const ECSManager&) = delete;
    ECSManager& operator=(const ECSManager&) = delete;
    ECSManager(ECSManager&&) = default;
    ECSManager& operator=(ECSManager&&) = default;

    ~ECSManager() {
        // Systems and components may have references to ECS
        // They must be destroyed first
        std::cout << "Destroying ECS" << std::endl;
        ecm.clear();
        std::cout << "Components destroyed" << std::endl;
        systems.clear();
        std::cout << "Systems destroyed" << std::endl;
    }

    void addSystem(std::unique_ptr<System>&& system) {
        systems.push_back(std::move(system));
    }

    void iterate() {
        std::cout << "ECS iteration" << std::endl;
        for (auto& s : systems) {
            assert(s);
            s->run();
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
        return ecm.isValidEntity(e);
    }

    template <typename Component>
    bool has(Entity e) const {
        assert(isValidEntity(e));
        return ecm.has<Component>(e);
    }

    template <typename Component>
    bool hasAny() const {
        return ecm.hasAny<Component>();
    }

    template <typename Component>
    Component& get(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return ecm.get<Component>(e);
    }

    template <typename Component>
    const Component& get(Entity e) const {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return ecm.get<Component>(e);
    }

    template<typename... Components>
    Entity createEntity(Components&&... components) {
        Entity e = ecm.createEntity();
        (ecm.set(e, std::forward<Components>(components)), ...);
        return e;
    }

    void destroyEntity(Entity e) {
        assert(isValidEntity(e));
        ecm.destroyEntity(e);
    }

    template <typename Component>
    void set(Entity e, Component&& component) {
        assert(isValidEntity(e));
        ecm.set(e, std::move(component));
    }

    template <typename Component>
    void remove(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        ecm.remove<Component>(e);
    }

    template <typename Component>
    void removeAll() {
        ecm.removeAll<Component>();
    }

    template<typename... Components>
    EntityView view() {
        return ecm.view<Components...>();
    }

private:
    bool running = false;
    EntityComponentManager ecm;
    std::vector<std::unique_ptr<System>> systems;
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


class WindowMessageHandler {
public:
    virtual ~WindowMessageHandler() = default;
    virtual LRESULT handleWindowMessage(SysWindowMessage message) noexcept = 0;
};


class Window {
public:
    Window(Rect<int> location, std::unique_ptr<WindowMessageHandler>&& handler)
    : handler_(std::move(handler)) {
        hWnd_ = CreateWindowEx(
            NULL,
            getWindowClass(),
            L"Istok",
            WS_OVERLAPPEDWINDOW,
            location.left, location.top,
            location.right - location.left,
            location.bottom - location.top,
            NULL, NULL, getHInstance(), nullptr);
        if (!hWnd_) {
            throw std::runtime_error("Cannot create window");
        }
        Istok::GUI::WinAPI::prepareForGL(hWnd_);
        ShowWindow(hWnd_, SW_SHOW);
        SetWindowLongPtr(
            hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(handler_.get()));
        std::cout << "Window created: " << hWnd_ << std::endl;
    }
    
    ~Window() {
        if (hWnd_) {
            std::cout << "Destroying window: " << hWnd_ << std::endl;
            SetWindowLongPtr(hWnd_, GWLP_USERDATA, NULL);
            DestroyWindow(hWnd_);
            std::cout << "Window destroyed: " << hWnd_ << std::endl;
        }
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& source) = delete;
    Window& operator=(Window&& source) = delete;

    HWND getHWnd() const {
        return hWnd_;
    }

private:
    HWND hWnd_ = nullptr;
    std::unique_ptr<WindowMessageHandler> handler_;

    static LPCWSTR getWindowClass() {
        static WindowClass wc(windowProc, L"Istok");
        return wc.get();
    }

    static LRESULT CALLBACK windowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
    ) noexcept {
        if (WindowMessageHandler* handler =
                reinterpret_cast<WindowMessageHandler*>(
                    GetWindowLongPtr(hWnd, GWLP_USERDATA))
        ) {
            return handler->handleWindowMessage(
                SysWindowMessage(hWnd, msg, wParam, lParam));
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
};

using WindowPtr = std::unique_ptr<Window>;

class WindowEntityMessageHandler {
public:
    virtual ~WindowEntityMessageHandler() = default;
    virtual LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message) noexcept = 0;
};


class CloseHandler : public WindowEntityMessageHandler {
public:
    CloseHandler(ECSManager& ecs) : ecs(ecs) {}
    
    LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message
    ) noexcept override {
        assert(message.msg == WM_CLOSE);
        if (ecs.has<WindowHandler::Close>(entity)) {
            std::cout << "handler: WM_CLOSE " <<
                entity.value << std::endl;
            ecs.get<WindowHandler::Close>(entity).func();
            return 0;
        }
        return handleByDefault(message);
    }

private:
    ECSManager& ecs;
};


class SizeHandler : public WindowEntityMessageHandler {
public:
    SizeHandler(ECSManager& ecs) : ecs(ecs) {}
    
    LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message
    ) noexcept override {
        assert(message.msg == WM_SIZE);
        if (ecs.has<std::unique_ptr<Window>>(entity)) {
            std::cout << "message: WM_SIZE " <<
                entity.value << std::endl;
            ecs.iterate();
        }
        return 0;
    }

private:
    ECSManager& ecs;
};


class Handler : public WindowEntityMessageHandler {
public:
    Handler(ECSManager& ecs) {
        handlers[WM_CLOSE] = std::make_unique<CloseHandler>(ecs);
        handlers[WM_SIZE] = std::make_unique<SizeHandler>(ecs);
    }
    
    LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message
    ) noexcept override {
        if (handlers.contains(message.msg)) {
            idle = false;
            return handlers[message.msg]->handleWindowMessage(entity, message);
        }
        return handleByDefault(message);
    }

    bool isIdle() const {
        return idle;
    }

    void setIdle(bool value) {
        idle = value;
    }

private:
    std::unordered_map<
        UINT,
        std::unique_ptr<WindowEntityMessageHandler>
    > handlers;
    
    bool idle;
};


class WindowEntityMessageHandlerProxy : public WindowMessageHandler {
public:
    WindowEntityMessageHandlerProxy(
        Entity entity, WindowEntityMessageHandler& handler
    ) : entity_(entity), handler_(handler) {}
    
    LRESULT handleWindowMessage(SysWindowMessage message) noexcept override {
        return handler_.handleWindowMessage(entity_, message);
    }

private:
    Entity entity_;
    WindowEntityMessageHandler& handler_;
};


class WindowSystem: public System {
public:
    WindowSystem(ECSManager& ecs) : ecs_(ecs), handler_(ecs) {}

    void run() override {
        createMissingWindows();
        createGL();
        processMessages();
    }

private:
    ECSManager& ecs_;
    Handler handler_;
    Istok::GUI::WinAPI::GLContext gl_;

    void createMissingWindows() {
        for (auto& w : ecs_.view<ScreenLocation>()
            .exclude<std::unique_ptr<Window>>()
        ) {
            std::cout << "Creating window for entity " << w.value << std::endl;
            Rect<int>& location = ecs_.get<ScreenLocation>(w).value;
            ecs_.set(w, std::make_unique<Window>(
                location,
                std::make_unique<WindowEntityMessageHandlerProxy>(
                    w, handler_)));
        }
    }

    bool createGL() {
        if (gl_) {
            return true;
        }
        auto windows = ecs_.view<WindowPtr>();
        if (windows.begin() == windows.end()) {
            return false;
        }
        Entity window = *windows.begin();
        gl_ = Istok::GUI::WinAPI::GLContext(
            ecs_.get<WindowPtr>(window)->getHWnd());
        return true;
    }

    void processMessages() {
        while (handler_.isIdle()) {
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                ecs_.stop();
                return;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        handler_.setIdle(true);
    }
};


int main() {
    ECSManager ecs;
    ecs.addSystem(std::make_unique<WindowSystem>(ecs));
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
