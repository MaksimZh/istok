// ecsbased.cpp
#include <ecs.hpp>
#include <gui/winapi/wgl.hpp>
#include <gui/gl/buffer.hpp>
#include <logging.hpp>

#include <windows.h>
#include <dwmapi.h>
#include <unordered_set>
#include <functional>

using namespace Istok::ECS;
using namespace Istok::GUI;

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
    CLASS_WITH_LOGGER("Components.Windows");
    
    HWND hWnd_ = nullptr;

    void drop() {
        hWnd_ = nullptr;
    }

    void clear() {
        if (hWnd_) {
            LOG_DEBUG("Destroying window: {}", (void*)hWnd_);
            setHandler(nullptr);
            DestroyWindow(hWnd_);
            LOG_TRACE("Window destroyed: {}", (void*)hWnd_);
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
            LOG_DEBUG("Creating window for entity {}", w.value);
            Rect<int>& location = ecs_.get<ScreenLocation>(w).value;
            ecs_.set(w, WndHandle(createWindow(location)));
        }
    }

private:
    CLASS_WITH_LOGGER("Components.Windows");

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


void enableTransparency(HWND hWnd) {
    DWM_BLURBEHIND bb = { 0 };
    HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = hRgn;
    bb.fEnable = TRUE;
    DwmEnableBlurBehindWindow(hWnd, &bb);
}


class InitGLSystem: public System {
public:
    InitGLSystem(ECSManager& ecs) : ecs_(ecs) {}
    
    ~InitGLSystem() {
        ecs_.removeAll<WinAPI::GLContext>();
    }

    InitGLSystem(const InitGLSystem&) = delete;
    InitGLSystem& operator=(const InitGLSystem&) = delete;
    InitGLSystem(InitGLSystem&&) = delete;
    InitGLSystem& operator=(InitGLSystem&&) = delete;

    void run() override {
        for (auto& w : ecs_.view<NewWindowFlag, WndHandle>()) {
            HWND hWnd = ecs_.get<WndHandle>(w).getHWnd();
            enableTransparency(hWnd);
            WinAPI::prepareForGL(hWnd);
            if (!ecs_.hasAny<WinAPI::GLContext>()) {
                ecs_.createEntity(WinAPI::GLContext(hWnd));
            }
        }
    }

private:
    ECSManager& ecs_;
};


class SceneSystem: public System {
public:
    SceneSystem(ECSManager& ecs) : ecs_(ecs) {}
    
    ~SceneSystem() {
        if (!ecs_.hasAny<WinAPI::GLContext>()) {
            return;
        }
        WinAPI::GLContext& gl = ecs_.get<WinAPI::GLContext>(
            *ecs_.view<WinAPI::GLContext>().begin());
    }

    SceneSystem(const SceneSystem&) = delete;
    SceneSystem& operator=(const SceneSystem&) = delete;
    SceneSystem(SceneSystem&&) = delete;
    SceneSystem& operator=(SceneSystem&&) = delete;

    void run() override {
        for (auto& w : ecs_.view<NewWindowFlag, WndHandle>()) {
            HWND hWnd = ecs_.get<WndHandle>(w).getHWnd();
            enableTransparency(hWnd);
            WinAPI::prepareForGL(hWnd);
            if (!ecs_.hasAny<WinAPI::GLContext>()) {
                ecs_.createEntity(WinAPI::GLContext(hWnd));
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
protected:
    CLASS_WITH_LOGGER("Components.Windows");
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
        LOG_TRACE("handler: WM_CLOSE {}", entity.value);
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
        LOG_TRACE("handler: WM_SIZE {}", entity.value);
        HWND hWnd = ecs_.get<WndHandle>(entity).getHWnd();
        InvalidateRect(hWnd, NULL, FALSE);
        ecs_.iterate();
        return 0;
    }

private:
    ECSManager& ecs_;
};


using VertexBuffer = OpenGL::VertexBuffer<WinAPI::WGL>;
using VertexArray = OpenGL::VertexArray<WinAPI::WGL>;


class PaintHandler : public WindowEntityMessageHandler {
public:
    PaintHandler(ECSManager& ecs) : ecs_(ecs) {}
    
    LRESULT handleWindowMessage(
        Entity entity, SysWindowMessage message
    ) noexcept override {
        assert(message.msg == WM_PAINT);
        if (!ecs_.hasAny<WinAPI::GLContext>() ||
            !ecs_.has<WndHandle>(entity) ||
            !ecs_.has<VertexBuffer>(entity) ||
            !ecs_.has<VertexArray>(entity)
        ) {
            return handleByDefault(message);
        }
        LOG_TRACE("message: WM_PAINT {}", entity.value);
        WinAPI::GLContext& gl = ecs_.get<WinAPI::GLContext>(
            *ecs_.view<WinAPI::GLContext>().begin());
        HWND hWnd = message.hWnd;
        WinAPI::WGL::Scope scope(gl, hWnd);
        RECT rect;
        GetClientRect(hWnd, &rect);
        glViewport(0, 0, rect.right, rect.bottom);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        VertexArray& vao = ecs_.get<VertexArray>(entity);
        vao.bind(scope);
        VertexBuffer& vbo = ecs_.get<VertexBuffer>(entity);
        vbo.bind(scope);
        glDrawArrays(GL_TRIANGLES, 0, 3);
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
    CLASS_WITH_LOGGER("Components.Windows");
    
    ECSManager& ecs_;
    Handler handler_;

    void attachHandler() {
        for (auto& w : ecs_.view<NewWindowFlag, WndHandle>()) {
            LOG_DEBUG("Set handler for window {}", w.value);
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
    SET_LOGGER("", Istok::Logging::terminal, Istok::Logging::Level::all);
    WITH_LOGGER("");
    LOG_TRACE("App: begin");
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
    ecs.run();
    ecs.clear();
    LOG_TRACE("App: end");
    return 0;
}
