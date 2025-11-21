#include <ecs.hpp>
#include <gui/winapi/wgl.hpp>

#include <windows.h>
#include <iostream>
#include <unordered_set>
#include <functional>

using namespace Istok::ECS;

class ECSManager {
public:
    ECSManager() {}

    ECSManager(const ECSManager&) = delete;
    ECSManager& operator=(const ECSManager&) = delete;
    ECSManager(ECSManager&&) = default;
    ECSManager& operator=(ECSManager&&) = default;

    /* Calls clear method to ensure the reference to ECSManager is valid
     * until all systems and components are destroyed.
     */
    ~ECSManager() {
        clear();
    }

    void pushSystem(std::unique_ptr<System>&& system) {
        if (!system) {
            throw std::runtime_error("Null system pointer");
        }
        systems_.push(std::move(system));
    }

    void popSystem() {
        if (systems_.empty()) {
            throw std::runtime_error("No system to pop");
        }
        systems_.pop();
    }

    bool hasSystems() const {
        return systems_.empty();
    }

    /* The systems are destroyed first in reverse order (stack pop until empty).
     * The components are destroyed after the systems in undefined order.
     */
    void clear() {
        systems_.clear();
        ecm_.clear();
    }

    void iterate() {
        std::cout << "ECS iteration" << std::endl;
        systems_.run();
    }

    void run() {
        if (running_) {
            return;
        }
        running_ = true;
        while (running_) {
            iterate();
        }
    }

    void stop() {
        running_ = false;
    }

    bool isValidEntity(Entity e) const {
        return ecm_.isValidEntity(e);
    }

    template <typename Component>
    bool has(Entity e) const {
        assert(isValidEntity(e));
        return ecm_.has<Component>(e);
    }

    template <typename Component>
    bool hasAny() const {
        return ecm_.hasAny<Component>();
    }

    template <typename Component>
    Component& get(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return ecm_.get<Component>(e);
    }

    template <typename Component>
    const Component& get(Entity e) const {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return ecm_.get<Component>(e);
    }

    template<typename... Components>
    Entity createEntity(Components&&... components) {
        Entity e = ecm_.createEntity();
        (ecm_.set(e, std::forward<Components>(components)), ...);
        return e;
    }

    void destroyEntity(Entity e) {
        assert(isValidEntity(e));
        ecm_.destroyEntity(e);
    }

    template <typename Component>
    void set(Entity e, Component&& component) {
        assert(isValidEntity(e));
        ecm_.set(e, std::move(component));
    }

    template <typename Component>
    void remove(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        ecm_.remove<Component>(e);
    }

    template <typename Component>
    void removeAll() {
        ecm_.removeAll<Component>();
    }

    template<typename... Components>
    EntityView view() {
        return ecm_.view<Components...>();
    }

private:
    bool running_ = false;
    EntityComponentManager ecm_;
    SystemStack systems_;
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


template <typename Component>
Component& requestUnique(ECSManager& ecs) {
    Entity entity = ecs.hasAny<Component>()
        ? *ecs.view<Component>().begin()
        : ecs.createEntity(Component());
    return ecs.get<Component>(entity);
}


struct WindowsIdle {
    bool value;
};


class CloseHandler : public WindowEntityMessageHandler {
public:
    CloseHandler(ECSManager& ecs) : ecs_(ecs) {}
    
    LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message
    ) noexcept override {
        assert(message.msg == WM_CLOSE);
        if (!ecs_.hasAny<WindowsIdle>() ||
            !ecs_.has<WindowHandler::Close>(entity)
        ) {
            return handleByDefault(message);
        }
        requestUnique<WindowsIdle>(ecs_).value = false;
        std::cout << "handler: WM_CLOSE " <<
            entity.value << std::endl;
        ecs_.get<WindowHandler::Close>(entity).func();
        return 0;
    }

private:
    ECSManager& ecs_;
};


class SizeHandler : public WindowEntityMessageHandler {
public:
    SizeHandler(ECSManager& ecs) : ecs_(ecs) {}
    
    LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message
    ) noexcept override {
        assert(message.msg == WM_SIZE);
        if (!ecs_.hasAny<WindowsIdle>() ||
            !ecs_.has<std::unique_ptr<Window>>(entity)
        ) {
            return handleByDefault(message);
        }
        requestUnique<WindowsIdle>(ecs_).value = false;
        std::cout << "message: WM_SIZE " <<
            entity.value << std::endl;
        ecs_.iterate();
        return 0;
    }

private:
    ECSManager& ecs_;
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
            return handlers[message.msg]->handleWindowMessage(entity, message);
        }
        return handleByDefault(message);
    }

private:
    std::unordered_map<
        UINT,
        std::unique_ptr<WindowEntityMessageHandler>
    > handlers;
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
    ~WindowSystem() {
        ecs_.removeAll<std::unique_ptr<Window>>();
    }

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
        if (!ecs_.hasAny<WindowsIdle>()) {
            requestUnique<WindowsIdle>(ecs_).value = true;
        }
        while (requestUnique<WindowsIdle>(ecs_).value) {
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                ecs_.stop();
                return;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        requestUnique<WindowsIdle>(ecs_).value = true;
    }
};


int main() {
    ECSManager ecs;
    ecs.pushSystem(std::make_unique<WindowSystem>(ecs));
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
