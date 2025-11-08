#include <ecs.hpp>

#include <windows.h>
#include <iostream>
#include <unordered_set>

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


LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    if (msg == WM_CLOSE) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}


class Window {
public:
    Window(const Rect<int>& location) {
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
        ShowWindow(hWnd, SW_SHOW);
    }
    
    ~Window() {
        if (hWnd) {
            DestroyWindow(hWnd);
        }
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = default;
    Window& operator=(Window&&) = default;

private:
    HWND hWnd;

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
    Entity menu = ecm.createEntity();
    ecm.set(menu, NewWindow{});
    ecm.set(menu, ScreenLocation{{300, 200, 500, 500}});
    for (auto& w : ecm.view<NewWindow, ScreenLocation>()) {
        std::cout << "Creating window for entity " << w.value << std::endl;
        ecm.set(w, std::move(Window(ecm.get<ScreenLocation>(w).value)));
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
    ecm.removeAll<Window>();
    return 0;
}
