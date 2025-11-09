#include <ecs.hpp>

#include <windows.h>
#include <iostream>
#include <unordered_set>
#include <functional>

using namespace Istok::ECS;

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

struct NewWindow {};

namespace WindowHandler {

struct Close {
    std::function<void()> func;
};

} // namespace WindowHandler


struct ECSBinding {
    EntityComponentManager* ecm;
    Entity entity;
};


LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    if (msg == WM_CLOSE) {
        if (ECSBinding* binding = reinterpret_cast<ECSBinding*>(
            GetWindowLongPtr(hWnd, GWLP_USERDATA))
        ) {
            if (binding->ecm->has<WindowHandler::Close>(binding->entity)) {
                binding->ecm->get<WindowHandler::Close>(binding->entity).func();
                return 0;
            }
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}


class Window {
public:
    Window(
        EntityComponentManager& ecm,
        Entity entity,
        const Rect<int>& location
    ) : binding(std::make_unique<ECSBinding>(&ecm, entity)) {
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
            hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(binding.get()));
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
    std::unique_ptr<ECSBinding> binding;
    std::unique_ptr<HWND> hWnd;

    static LPCWSTR getWindowClass() {
        static WindowClass wc(windowProc, L"Istok");
        return wc.get();
    }
};


int main() {
    EntityComponentManager ecm;
    Entity window = ecm.createEntity();
    ecm.set(window, NewWindow{});
    ecm.set(window, ScreenLocation{{200, 100, 600, 400}});
    ecm.set(window, WindowHandler::Close{[&](){ PostQuitMessage(0); }});
    Entity menu = ecm.createEntity();
    ecm.set(menu, NewWindow{});
    ecm.set(menu, ScreenLocation{{300, 200, 500, 500}});
    ecm.set(menu, WindowHandler::Close{[&](){ ecm.destroyEntity(menu); }});
    for (auto& w : ecm.view<NewWindow, ScreenLocation>()) {
        std::cout << "Creating window for entity " << w.value << std::endl;
        ecm.set(w, std::move(
            Window(ecm, w, ecm.get<ScreenLocation>(w).value)));
    }
    ecm.removeAll<NewWindow>();
    while (true) {
        MSG msg;
        GetMessage(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
