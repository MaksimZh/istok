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


class WndHandle {
public:
    WndHandle() = default;
    
    WndHandle(HWND hWnd) : hWnd_(hWnd) {}

    ~WndHandle() {
        clear();
    }

    WndHandle(const WndHandle&) = delete;
    WndHandle& operator=(const WndHandle&) = delete;
    
    WndHandle(WndHandle&& source) : hWnd_(source.hWnd_) {
        source.drop();
    }
    
    WndHandle& operator=(WndHandle&& source) {
        if (this != &source) {
            clear();
            hWnd_ = source.hWnd_;
            source.drop();
        }
        return *this;
    }

    operator bool() const noexcept {
        return hWnd_;
    }

    HWND getHWnd() const {
        return hWnd_;
    }

    void setHandler(WindowMessageHandler* handler) {
        SetWindowLongPtr(
            hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(handler));
    }

private:
    HWND hWnd_ = nullptr;

    void drop() {
        hWnd_ = nullptr;
    }

    void clear() {
        if (hWnd_) {
            setHandler(nullptr);
            std::cout << "Destroying window: " << hWnd_ << std::endl;
            DestroyWindow(hWnd_);
            std::cout << "Window destroyed: " << hWnd_ << std::endl;
            drop();
        }
    }
};


struct NewWindowFlag {};

class CreateWindowsSystem : public System {
public:
    CreateWindowsSystem(ECSManager& ecs) : ecs_(ecs) {}
    
    ~CreateWindowsSystem() {
        ecs_.removeAll<WndHandle>();
    }

    CreateWindowsSystem(const CreateWindowsSystem&) = delete;
    CreateWindowsSystem& operator=(const CreateWindowsSystem&) = delete;
    CreateWindowsSystem(CreateWindowsSystem&&) = delete;
    CreateWindowsSystem& operator=(CreateWindowsSystem&&) = delete;

    void run() override {
        for (auto& w : ecs_.view<NewWindowFlag, ScreenLocation>()) {
            std::cout << "Creating window for entity " << w.value << std::endl;
            Rect<int>& location = ecs_.get<ScreenLocation>(w).value;
            ecs_.set(w, WndHandle(createWindow(location)));
        }
    }

private:
    ECSManager& ecs_;

    static HWND createWindow(Rect<int> location) {
        static WindowClass windowClass(windowProc, L"Istok");
        HWND hWnd = CreateWindowEx(
            NULL,
            windowClass.get(),
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
        return hWnd;
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


using Istok::GUI::WinAPI::DCHandle;
using Istok::GUI::WinAPI::GLContext;
using Istok::GUI::WinAPI::WGL;


class InitGLSystem: public System {
public:
    InitGLSystem(ECSManager& ecs) : ecs_(ecs) {}
    
    ~InitGLSystem() {}

    InitGLSystem(const InitGLSystem&) = delete;
    InitGLSystem& operator=(const InitGLSystem&) = delete;
    InitGLSystem(InitGLSystem&&) = delete;
    InitGLSystem& operator=(InitGLSystem&&) = delete;

    void run() override {
        for (auto& w : ecs_.view<NewWindowFlag, WndHandle>()) {
            HWND hWnd = ecs_.get<WndHandle>(w).getHWnd();
            Istok::GUI::WinAPI::prepareForGL(hWnd);
            if (!ecs_.hasAny<GLContext>()) {
                ecs_.createEntity(GLContext(hWnd));
            }
        }
    }

private:
    ECSManager& ecs_;
};


class WindowEntityMessageHandler {
public:
    virtual ~WindowEntityMessageHandler() = default;
    virtual LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message) noexcept = 0;
};


class CloseHandler : public WindowEntityMessageHandler {
public:
    CloseHandler(ECSManager& ecs) : ecs_(ecs) {}
    
    LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message
    ) noexcept override {
        assert(message.msg == WM_CLOSE);
        if (!ecs_.has<WindowHandler::Close>(entity)) {
            return handleByDefault(message);
        }
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
        if (!ecs_.has<WndHandle>(entity)) {
            return handleByDefault(message);
        }
        std::cout << "message: WM_SIZE " <<
            entity.value << std::endl;
        ecs_.iterate();
        return 0;
    }

private:
    ECSManager& ecs_;
};


class PaintHandler : public WindowEntityMessageHandler {
public:
    PaintHandler(ECSManager& ecs) : ecs_(ecs) {}
    
    LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message
    ) noexcept override {
        assert(message.msg == WM_PAINT);
        if (!ecs_.hasAny<GLContext>()) {
            return handleByDefault(message);
        }
        std::cout << "message: WM_PAINT " <<
            entity.value << std::endl;
        GLContext& gl = ecs_.get<GLContext>(*ecs_.view<GLContext>().begin());
        HWND hWnd = message.hWnd;
        WGL::Scope scope(gl, hWnd);
        RECT rect;
        GetClientRect(hWnd, &rect);
        glViewport(0, 0, rect.right, rect.bottom);
        glClearColor(0, 0, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        PAINTSTRUCT ps;
        SwapBuffers(BeginPaint(hWnd, &ps));
        EndPaint(hWnd, &ps);
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
        handlers[WM_PAINT] = std::make_unique<PaintHandler>(ecs);
    }
    
    LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message
    ) noexcept override {
        if (handlers.contains(message.msg)) {
            idle_ = false;
            return handlers[message.msg]->handleWindowMessage(entity, message);
        }
        return handleByDefault(message);
    }

    bool isIdle() const noexcept {
        return idle_;
    }

    void makeIdle() {
        idle_ = true;
    }

private:
    std::unordered_map<
        UINT,
        std::unique_ptr<WindowEntityMessageHandler>
    > handlers;
    bool idle_ = true;
};


class WindowEntityMessageHandlerProxy : public WindowMessageHandler {
public:
    WindowEntityMessageHandlerProxy(
        Entity entity, WindowEntityMessageHandler& handler
    ) : entity_(entity), handler_(&handler) {}
    
    LRESULT handleWindowMessage(SysWindowMessage message) noexcept override {
        return handler_->handleWindowMessage(entity_, message);
    }

private:
    Entity entity_;
    WindowEntityMessageHandler* handler_;
};


class WindowMessageSystem: public System {
public:
    WindowMessageSystem(ECSManager& ecs) : ecs_(ecs), handler_(ecs) {}

    ~WindowMessageSystem() {
        for (auto& w : ecs_.view<WndHandle>()) {
            ecs_.get<WndHandle>(w).setHandler(nullptr);
        }
    }

    WindowMessageSystem(const WindowMessageSystem&) = delete;
    WindowMessageSystem& operator=(const WindowMessageSystem&) = delete;
    WindowMessageSystem(WindowMessageSystem&&) = delete;
    WindowMessageSystem& operator=(WindowMessageSystem&&) = delete;

    void run() override {
        attachHandler();
        finishWindowInitialization();
        runMessageLoop();
    }

private:
    ECSManager& ecs_;
    Handler handler_;

    void attachHandler() {
        for (auto& w : ecs_.view<NewWindowFlag, WndHandle>()) {
            std::cout << "Set handler for window " << w.value << std::endl;
            ecs_.set(w, std::make_unique<WindowEntityMessageHandlerProxy>(w, handler_));
            ecs_.get<WndHandle>(w).setHandler(
                ecs_.get<std::unique_ptr<WindowEntityMessageHandlerProxy>>(w).get());
        }
    }

    void finishWindowInitialization() {
        ecs_.removeAll<NewWindowFlag>();
    }

    void runMessageLoop() {
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
        handler_.makeIdle();
    }
};


int main() {
    ECSManager ecs;
    ecs.pushSystem(std::make_unique<CreateWindowsSystem>(ecs));
    ecs.pushSystem(std::make_unique<InitGLSystem>(ecs));
    ecs.pushSystem(std::make_unique<WindowMessageSystem>(ecs));
    ecs.createEntity(
        NewWindowFlag{},
        ScreenLocation{{200, 100, 600, 400}},
        WindowHandler::Close{[&](){
            ecs.createEntity(
                NewWindowFlag{},
                ScreenLocation{{300, 200, 500, 500}},
                WindowHandler::Close{[&](){ ecs.stop(); }});
        }});
    std::cout << "Run" << std::endl;
    ecs.run();
    std::cout << "Stopped" << std::endl;
    ecs.clear();
    std::cout << "Clean" << std::endl;
    return 0;
}
