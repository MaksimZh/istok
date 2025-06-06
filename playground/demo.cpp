#include <windows.h>
#include <stdexcept>
#include <string>

using namespace std;


LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

class WndClassHandler {
public:
    WndClassHandler() = default;

    WndClassHandler(
        UINT style,
        WNDPROC lpfnWndProc,
        HINSTANCE hInstance,
        LPCWSTR className,
        int cbClsExtra = 0,
        int cbWndExtra = 0,
        HICON hIcon = nullptr,
        HCURSOR hCursor = nullptr,
        HBRUSH hbrBackground = nullptr,
        LPCWSTR lpszMenuName = nullptr,
        HICON hIconSm = nullptr
    ) : hInstance(hInstance), name(className) {
        WNDCLASSEX wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = style;
        wcex.lpfnWndProc = lpfnWndProc;
        wcex.cbClsExtra = cbClsExtra;
        wcex.cbWndExtra = cbWndExtra;
        wcex.hInstance = hInstance;
        wcex.hIcon = hIcon;
        wcex.hCursor = hCursor;
        wcex.hbrBackground = hbrBackground;
        wcex.lpszMenuName = lpszMenuName;
        wcex.lpszClassName = className;
        wcex.hIconSm = hIconSm;
        if (!RegisterClassEx(&wcex)) {
            throw std::runtime_error("Failed to register window class.");
        }
    }

    WndClassHandler(const WndClassHandler&) = delete;
    WndClassHandler& operator=(const WndClassHandler&) = delete;

    WndClassHandler(WndClassHandler&& other) noexcept
        : hInstance(other.hInstance), name(other.name) {
        other.drop();
    }

    WndClassHandler& operator=(WndClassHandler&& other) noexcept {
        if (this != &other) {
            clean();
            hInstance = other.hInstance;
            name = other.name;
            other.drop();
        }
        return *this;
    }

    ~WndClassHandler() {
        clean();
    }

    operator bool() const {
        return name != nullptr;
    }

    LPCWSTR get() const {
        return name;
    }

    operator LPCWSTR() const {
        return name;
    }


private:
    HINSTANCE hInstance = nullptr;
    LPCWSTR name = nullptr;

    void drop() {
        hInstance = nullptr;
        name = nullptr;
    }

    void clean() {
        if (!*this) return;
        UnregisterClass(name, hInstance);
        drop();
    }
};


class WndHandler {
public:
    WndHandler() = default;

    WndHandler(
        DWORD dwExStyle,
        LPCWSTR lpClassName,
        LPCWSTR lpWindowName,
        DWORD dwStyle,
        int x,
        int y,
        int nWidth,
        int nHeight,
        HWND hWndParent,
        HMENU hMenu,
        HINSTANCE hInstance,
        LPVOID lpParam
    ) : hWnd(CreateWindowEx(
                dwExStyle, lpClassName, lpWindowName, dwStyle,
                x, y, nWidth, nHeight,
                hWndParent, hMenu, hInstance, lpParam)) {
        if (!hWnd) {
            throw std::runtime_error("Failed to create window.");
        }
    }

    ~WndHandler() {
        clean();
    }

    WndHandler(const WndHandler&) = delete;
    WndHandler& operator=(const WndHandler&) = delete;

    WndHandler(WndHandler&& other) noexcept
        : hWnd(other.hWnd) {
        other.drop();
    }

    WndHandler& operator=(WndHandler&& other) noexcept {
        if (this != &other) {
            clean();
            hWnd = other.hWnd;
            other.drop();
        }
        return *this;
    }

    operator bool() const {
        return hWnd != nullptr;
    }

    HWND get() const {
        return hWnd;
    }

    operator HWND() const {
        return hWnd;
    }


private:
    HWND hWnd = nullptr;

    void drop() {
        hWnd = nullptr;
    }

    void clean() {
        if (!*this) return;
        DestroyWindow(hWnd);
        drop();
    }
};


class DCHandler {
public:
    DCHandler(HDC hdc) : hDC(hdc) {}

    ~DCHandler() {
        clean();
    }

    DCHandler(const DCHandler&) = delete;
    DCHandler& operator=(const DCHandler&) = delete;

    DCHandler(DCHandler&& other) noexcept
        : hDC(other.hDC) {
        other.drop();
    }

    DCHandler& operator=(DCHandler&& other) noexcept {
        if (this != &other) {
            clean();
            hDC = other.hDC;
            other.drop();
        }
        return *this;
    }

    operator bool() const {
        return hDC != nullptr;
    }

    HDC get() const {
        return hDC;
    }

    operator HDC() const {
        return hDC;
    }

private:
    HDC hDC = nullptr;

    void drop() {
        hDC = nullptr;
    }

    void clean() {
        if (*this) {
            ReleaseDC(WindowFromDC(hDC), hDC);
        }
        drop();
    }
};


int main() {
    HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
        
    WndClassHandler wc(CS_OWNDC, windowProc, hInstance, L"Istok");
    WndHandler wnd(
        NULL, wc, L"Istok Demo", WS_OVERLAPPEDWINDOW,
        100, 200, 400, 300,
        NULL, NULL, hInstance, nullptr);
    ShowWindow(wnd, SW_SHOW);
    
    while (true) {
        MSG msg;
        GetMessageW(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
