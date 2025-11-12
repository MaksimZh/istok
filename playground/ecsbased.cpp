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
    ECSManager() = default;
    ECSManager(const ECSManager&) = delete;
    ECSManager& operator=(const ECSManager&) = delete;
    ECSManager(ECSManager&&) = default;
    ECSManager& operator=(ECSManager&&) = default;

    ECSManager(std::initializer_list<System> args) : systems(args) {}

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
        return ecm.isValidEntity(e);
    }

    template <typename Component>
    bool has(Entity e) const {
        assert(isValidEntity(e));
        return ecm.has<Component>(e);
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

    BoundEntity bind(Entity e) {
        assert(isValidEntity(e));
        return BoundEntity(ecm, e);
    }
    
    template<typename... Components>
    BoundEntity createBoundEntity(Components&&... components) {
        Entity e = createEntity(std::forward<Components>(components)...);
        return bind(e);
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


struct WindowData {
    ECSManager* ecs;
    Entity entity;
};


LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;


class Window {
public:
    Window(WindowData data) : data(std::make_unique<WindowData>(data)) {
        auto entity = data.ecs->bind(data.entity);
        auto location = entity.get<ScreenLocation>().value;
        HWND hWnd = CreateWindowEx(
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
        SetWindowLongPtr(
            hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(
                this->data.get()));
        ShowWindow(hWnd, SW_SHOW);
        std::cout << "Create window " << hWnd << std::endl;
        this->hWnd = std::make_unique<HWND>(hWnd);
    }
    
    ~Window() {
        if (hWnd && *hWnd) {
            std::cout << "Destroy window " << *hWnd << std::endl;
            DestroyWindow(*hWnd);
        }
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = default;
    Window& operator=(Window&&) = default;

    HWND getHWnd() const {
        return *hWnd;
    }

private:
    std::unique_ptr<WindowData> data;
    std::unique_ptr<HWND> hWnd;

    static LPCWSTR getWindowClass() {
        static WindowClass wc(windowProc, L"Istok");
        return wc.get();
    }
};

enum class WindowState {
    IDLE,
    PROCESSING,
    CHANGED
};

LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    WindowData* data = reinterpret_cast<WindowData*>(
        GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!data) {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    auto stateView = data->ecs->view<WindowState>();
    if (stateView.begin() == stateView.end()) {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    auto global = data->ecs->bind(*stateView.begin());
    global.set(WindowState::IDLE);
    if (msg == WM_CLOSE) {
        global.set(WindowState::CHANGED);
        auto entity = data->ecs->bind(data->entity);
        if (entity.has<WindowHandler::Close>()) {
            entity.get<WindowHandler::Close>().func();
            return 0;
        }
    }
    if (msg == WM_SIZE) {
        global.set(WindowState::CHANGED);
        auto entity = data->ecs->bind(data->entity);
        if (entity.has<Window>()) {
            std::cout << "message: WM_SIZE" << std::endl;
            data->ecs->iterate();
        }
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}


void createMissingWindows(ECSManager& ecs) {
    for (auto& w : ecs.view<ScreenLocation>().exclude<Window>()) {
        std::cout << "Creating window for entity " << w.value << std::endl;
        ecs.set(w, Window(WindowData{&ecs, w}));
    }
}


void processWindowsMessages(ECSManager& ecs) {
    auto stateView = ecs.view<WindowState>();
    auto global = ecs.bind(
        (stateView.begin() == stateView.end())
            ? ecs.createEntity(WindowState::IDLE)
            : *stateView.begin());
    while (global.get<WindowState>() == WindowState::IDLE) {
        global.set(WindowState::PROCESSING);
        MSG msg;
        GetMessage(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            ecs.stop();
            return;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    global.set(WindowState::IDLE);
}


int main() {
    ECSManager ecs {
        System{createMissingWindows},
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
