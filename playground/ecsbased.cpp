// ecsbased.cpp
#include <ecs.hpp>
#include <gui/winapi/wgl.hpp>
#include <gui/winapi/window.hpp>
#include <gui/gl/buffer.hpp>
#include <logging.hpp>

#include <memory>
#include <windef.h>
#include <windows.h>
#include <dwmapi.h>
#include <functional>

using namespace Istok::ECS;
using namespace Istok::GUI;
using Istok::GUI::WinAPI::WndHandle;

template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};

struct ScreenLocation {
    Rect<int> value;
};

namespace WindowHandler {

struct Close {
    std::function<void()> func;
};

} // namespace WindowHandler


struct NewWindowFlag {};

class CreateWindowsSystem : public System {
public:
    CreateWindowsSystem(ECSManager& ecs) : ecs_(ecs) {
        LOG_DEBUG("create");
    }
    
    ~CreateWindowsSystem() {
        LOG_DEBUG("destroy");
        ecs_.removeAll<WndHandle>();
    }

    CreateWindowsSystem(const CreateWindowsSystem&) = delete;
    CreateWindowsSystem& operator=(const CreateWindowsSystem&) = delete;
    CreateWindowsSystem(CreateWindowsSystem&&) = delete;
    CreateWindowsSystem& operator=(CreateWindowsSystem&&) = delete;

    void run() override {
        LOG_DEBUG("run");
        for (auto& w : ecs_.view<NewWindowFlag>()) {
            Rect<int> location = ecs_.has<ScreenLocation>(w)
                ? ecs_.get<ScreenLocation>(w).value
                : Rect<int>{};
            HWND hWnd = createWindow(location);
            LOG_DEBUG("window {} created @{}", (void*)hWnd, w.value);
            ecs_.set(w, WndHandle(hWnd));
        }
    }

private:
    CLASS_WITH_LOGGER_PREFIX("Windows", "CreateWindowsSystem: ");

    ECSManager& ecs_;

    static HWND createWindow(Rect<int> location) {
        static WinAPI::WindowClass windowClass(windowProc, L"Istok");
        HWND hWnd = CreateWindowEx(
            NULL,
            windowClass.get(),
            L"Istok",
            WS_OVERLAPPEDWINDOW,
            location.left, location.top,
            location.right - location.left,
            location.bottom - location.top,
            NULL, NULL, WinAPI::getHInstance(), nullptr);
        if (!hWnd) {
            throw std::runtime_error("Cannot create window");
        }
        return hWnd;
    }

    static LRESULT CALLBACK windowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
    ) noexcept {
        if (WinAPI::WindowMessageHandler* handler =
                reinterpret_cast<WinAPI::WindowMessageHandler*>(
                    GetWindowLongPtr(hWnd, GWLP_USERDATA))
        ) {
            return handler->handleWindowMessage(
                WinAPI::WindowMessage(hWnd, msg, wParam, lParam));
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
};


struct ShowWindowFlag {};

class ShowWindowsSystem : public System {
public:
    ShowWindowsSystem(ECSManager& ecs) : ecs_(ecs) {
        LOG_DEBUG("create");
    }
    
    ~ShowWindowsSystem() {
        LOG_DEBUG("destroy");
        ecs_.removeAll<WndHandle>();
    }

    ShowWindowsSystem(const ShowWindowsSystem&) = delete;
    ShowWindowsSystem& operator=(const ShowWindowsSystem&) = delete;
    ShowWindowsSystem(ShowWindowsSystem&&) = delete;
    ShowWindowsSystem& operator=(ShowWindowsSystem&&) = delete;

    void run() override {
        LOG_DEBUG("run");
        for (auto& w : ecs_.view<ShowWindowFlag, WndHandle>()) {
            LOG_DEBUG("show window @{}", w.value);
            ShowWindow(ecs_.get<WndHandle>(w).getHWnd(), SW_SHOW);
        }
        ecs_.removeAll<ShowWindowFlag>();
    }

private:
    CLASS_WITH_LOGGER_PREFIX("Windows", "ShowWindowsSystem: ");

    ECSManager& ecs_;
};


void enableTransparency(HWND hWnd) {
    DWM_BLURBEHIND bb = { 0 };
    HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = hRgn;
    bb.fEnable = TRUE;
    DwmEnableBlurBehindWindow(hWnd, &bb);
}


struct GLHolderTag {};

class InitGLSystem: public System {
public:
    InitGLSystem(ECSManager& ecs) : ecs_(ecs) {
        LOG_DEBUG("create");
        ecs.createEntity(
            NewWindowFlag{},
            GLHolderTag{});
    }
    
    ~InitGLSystem() {
        LOG_DEBUG("destroy");
        ecs_.removeAll<WinAPI::GLContext>();
    }

    InitGLSystem(const InitGLSystem&) = delete;
    InitGLSystem& operator=(const InitGLSystem&) = delete;
    InitGLSystem(InitGLSystem&&) = delete;
    InitGLSystem& operator=(InitGLSystem&&) = delete;

    void run() override {
        LOG_DEBUG("run");
        for (auto& w : ecs_.view<NewWindowFlag, WndHandle>()) {
            HWND hWnd = ecs_.get<WndHandle>(w).getHWnd();
            LOG_DEBUG("prepare window for GL @{}", w.value);
            enableTransparency(hWnd);
            WinAPI::prepareForGL(hWnd);
        }
        if (!ecs_.hasAny<WinAPI::GLContext>()) {
            assert(ecs_.hasAny<GLHolderTag>());
            Entity e = *ecs_.view<GLHolderTag>().begin();
            assert(ecs_.has<WndHandle>(e));
            HWND hWnd = ecs_.get<WndHandle>(e).getHWnd();
            LOG_DEBUG(
                "creating GLContext @{} using window {}",
                e.value, (void*)hWnd);
            ecs_.set(e, WinAPI::GLContext(hWnd));
            LOG_DEBUG("GLContext created");
        }
    }

private:
    CLASS_WITH_LOGGER_PREFIX("GL", "InitGLSystem: ");
    
    ECSManager& ecs_;
};

using VertexBuffer = OpenGL::VertexBuffer<WinAPI::WGL>;
using VertexArray = OpenGL::VertexArray<WinAPI::WGL>;

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
        Entity entity, WinAPI::WindowMessage message) noexcept = 0;
protected:
    CLASS_WITH_LOGGER_PREFIX("Windows", "MSG: ");
};


class CloseHandler : public WindowEntityMessageHandler {
public:
    CloseHandler(ECSManager& ecs) : ecs_(ecs) {}
    
    LRESULT handleWindowMessage(
        Entity entity, WinAPI::WindowMessage message
    ) noexcept override {
        assert(message.msg == WM_CLOSE);
        if (!ecs_.has<WindowHandler::Close>(entity)) {
            LOG_TRACE("WM_CLOSE @{} -> skip", entity.value);
            return handleByDefault(message);
        }
        LOG_TRACE("WM_CLOSE @{} -> process", entity.value);
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
        Entity entity, WinAPI::WindowMessage message
    ) noexcept override {
        assert(message.msg == WM_SIZE);
        if (!ecs_.has<WndHandle>(entity)) {
            LOG_TRACE("WM_SIZE @{} -> skip", entity.value);
        }
        LOG_TRACE("WM_SIZE @{} -> process", entity.value);
        HWND hWnd = ecs_.get<WndHandle>(entity).getHWnd();
        InvalidateRect(hWnd, NULL, FALSE);
        ecs_.iterate();
        return 0;
    }

private:
    ECSManager& ecs_;
};

class WindowRenderer {
public:
    virtual ~WindowRenderer() = default;

    virtual void render(ECSManager& ecs, Entity entity) = 0;
};

class PaintHandler : public WindowEntityMessageHandler {
public:
    PaintHandler(ECSManager& ecs) : ecs_(ecs) {}

    LRESULT handleWindowMessage(
        Entity entity, WinAPI::WindowMessage message
    ) noexcept override {
        assert(message.msg == WM_PAINT);
        if (!ecs_.has<std::unique_ptr<WindowRenderer>>(entity)) {
            LOG_TRACE("WM_PAINT @{} -> skip", entity.value);
            return handleByDefault(message);
        }
        LOG_TRACE("WM_PAINT @{} -> process", entity.value);
        ecs_.get<std::unique_ptr<WindowRenderer>>(entity)
            ->render(ecs_, entity);
        validateWindow(message.hWnd);
        return 0;
    }

private:
    ECSManager& ecs_;

    static void validateWindow(HWND hWnd) {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
};


class ColorFillRenderer : public WindowRenderer {
public:
    ColorFillRenderer(float r, float g, float b, float a)
    : r_(r), g_(g), b_(b), a_(a) {}

    void render(ECSManager& ecs, Entity entity) override {
        if (
            !ecs.hasAny<WinAPI::GLContext>() ||
            !ecs.has<WndHandle>(entity)
        ) {
            return;
        }
        WinAPI::GLContext& gl = ecs.get<WinAPI::GLContext>(
            *ecs.view<WinAPI::GLContext>().begin());
        HWND hWnd = ecs.get<WndHandle>(entity).getHWnd();
        WinAPI::WGL::Scope scope(gl, hWnd);
        RECT rect;
        GetClientRect(hWnd, &rect);
        glViewport(0, 0, rect.right, rect.bottom);
        glClearColor(r_, g_, b_, a_);
        glClear(GL_COLOR_BUFFER_BIT);
        SwapBuffers(WinAPI::DCHandle(hWnd).get());
    }

private:
    float r_;
    float g_;
    float b_;
    float a_;
};


class DrawWindowSystem : public System {
public:
    DrawWindowSystem(ECSManager& ecs)
    : ecs_(ecs) {
        LOG_DEBUG("create");
    }

    ~DrawWindowSystem() {
        LOG_DEBUG("destroy");
        ecs_.removeAll<std::unique_ptr<WindowRenderer>>();
    }
    
    DrawWindowSystem(const DrawWindowSystem&) = delete;
    DrawWindowSystem& operator=(const DrawWindowSystem&) = delete;
    DrawWindowSystem(DrawWindowSystem&&) = delete;
    DrawWindowSystem& operator=(DrawWindowSystem&&) = delete;

    void run() override {
        LOG_DEBUG("run");
        for (auto& w : ecs_.view<NewWindowFlag>()) {
            LOG_DEBUG("set window renderer @{}", w.value);
            ecs_.set(
                w, std::unique_ptr<WindowRenderer>(
                std::make_unique<ColorFillRenderer>(0, 0, 0.5, 1)));
        }
    }

private:
    CLASS_WITH_LOGGER_PREFIX("Windows", "DrawWindowSystem: ");
  
    ECSManager& ecs_;
};

class SetGLForCleanupSystem : public System {
public:
    SetGLForCleanupSystem(ECSManager& ecs)
    : ecs_(ecs) {
        LOG_DEBUG("create");
    }

    ~SetGLForCleanupSystem() {
        LOG_DEBUG("destroy");
        assert(ecs_.hasAny<GLHolderTag>());
        Entity e = *ecs_.view<GLHolderTag>().begin();
        assert(ecs_.has<WinAPI::GLContext>(e));
        assert(ecs_.has<WndHandle>(e));
        HWND hWnd = ecs_.get<WndHandle>(e).getHWnd();
        ecs_.set(e, std::make_unique<WinAPI::DCHandle>(hWnd));
        LOG_DEBUG("make GLContext current @{}", e.value);
        ecs_.get<WinAPI::GLContext>(e).makeCurrent(
            *ecs_.get<std::unique_ptr<WinAPI::DCHandle>>(e));
    }
    
    SetGLForCleanupSystem(const SetGLForCleanupSystem&) = delete;
    SetGLForCleanupSystem& operator=(const SetGLForCleanupSystem&) = delete;
    SetGLForCleanupSystem(SetGLForCleanupSystem&&) = delete;
    SetGLForCleanupSystem& operator=(SetGLForCleanupSystem&&) = delete;

    void run() override {}

private:
    CLASS_WITH_LOGGER_PREFIX("Windows", "SetGLForCleanupSystem: ");
  
    ECSManager& ecs_;
};

class Handler : public WindowEntityMessageHandler {
public:
    using HandlerMap = std::unordered_map<
        UINT,
        std::unique_ptr<WindowEntityMessageHandler>>;
    
    Handler(HandlerMap handlers) : handlers_(std::move(handlers)) {}
    
    LRESULT handleWindowMessage(
        Entity entity, WinAPI::WindowMessage message
    ) noexcept override {
        if (handlers_.contains(message.msg)) {
            idle_ = false;
            return handlers_[message.msg]->handleWindowMessage(entity, message);
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
    HandlerMap handlers_;
    bool idle_ = true;
};


class WindowEntityMessageHandlerProxy : public WinAPI::WindowMessageHandler {
public:
    WindowEntityMessageHandlerProxy(
        Entity entity, WindowEntityMessageHandler& handler
    ) : entity_(entity), handler_(&handler) {}
    
    LRESULT handleWindowMessage(WinAPI::WindowMessage message) noexcept override {
        return handler_->handleWindowMessage(entity_, message);
    }

private:
    Entity entity_;
    WindowEntityMessageHandler* handler_;
};


class WindowMessageSystem : public System {
public:
    WindowMessageSystem(ECSManager& ecs, Handler::HandlerMap handlers)
    : ecs_(ecs), handler_(std::move(handlers)) {
        LOG_DEBUG("create");
    }

    ~WindowMessageSystem() {
        LOG_DEBUG("destroy");
        for (auto& w : ecs_.view<WndHandle>()) {
            ecs_.get<WndHandle>(w).setHandler(nullptr);
        }
    }

    WindowMessageSystem(const WindowMessageSystem&) = delete;
    WindowMessageSystem& operator=(const WindowMessageSystem&) = delete;
    WindowMessageSystem(WindowMessageSystem&&) = delete;
    WindowMessageSystem& operator=(WindowMessageSystem&&) = delete;

    void run() override {
        LOG_DEBUG("run");
        attachHandler();
        finishWindowInitialization();
        runMessageLoop();
    }

private:
    CLASS_WITH_LOGGER_PREFIX("Windows", "WindowMessageSystem: ");
    
    ECSManager& ecs_;
    Handler handler_;

    void attachHandler() {
        for (auto& w : ecs_.view<NewWindowFlag, WndHandle>()) {
            LOG_DEBUG("set message handler @{}", w.value);
            ecs_.set(
                w, std::make_unique<WindowEntityMessageHandlerProxy>(
                    w, handler_));
            ecs_.get<WndHandle>(w).setHandler(
                ecs_.get<std::unique_ptr<WindowEntityMessageHandlerProxy>>(
                    w).get());
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
    SET_LOGGER(
        "",
        Istok::Logging::TerminalLogger::GetInstance(),
        Istok::Logging::Level::all);
    WITH_LOGGER_PREFIX("", "App: ");
    LOG_TRACE("begin");
    
    ECSManager ecs;
    
    ecs.pushSystem(std::make_unique<CreateWindowsSystem>(ecs));
    ecs.pushSystem(std::make_unique<ShowWindowsSystem>(ecs));
    ecs.pushSystem(std::make_unique<InitGLSystem>(ecs));
    ecs.pushSystem(std::make_unique<DrawWindowSystem>(ecs));
    ecs.pushSystem(std::make_unique<SetGLForCleanupSystem>(ecs));

    Handler::HandlerMap handlerMap;
    handlerMap.emplace(WM_CLOSE, std::make_unique<CloseHandler>(ecs));
    handlerMap.emplace(WM_SIZE, std::make_unique<SizeHandler>(ecs));
    handlerMap.emplace(WM_PAINT, std::make_unique<PaintHandler>(ecs));
    ecs.pushSystem(
        std::make_unique<WindowMessageSystem>(ecs, std::move(handlerMap)));
    
    ecs.createEntity(
        NewWindowFlag{},
        ShowWindowFlag{},
        ScreenLocation{{200, 100, 600, 400}},
        WindowHandler::Close{[&](){
            ecs.createEntity(
                NewWindowFlag{},
                ShowWindowFlag{},
                ScreenLocation{{300, 200, 500, 500}},
                WindowHandler::Close{[&](){ ecs.stop(); }});
        }});
    
    ecs.run();
    
    ecs.clear();
    LOG_TRACE("end");
    return 0;
}
