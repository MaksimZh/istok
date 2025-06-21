#include <windows.h>
#include <stdexcept>
#include <string>
#include <optional>
#include <memory>
#include <map>

#include <gui/widget.hpp>


template <typename T>
struct Position {
    T x;
    T y;
};


template <typename T>
struct Size {
    T width;
    T height;
};


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
    DCHandler() = default;
    
    DCHandler(HWND hWnd) : hDC(GetWindowDC(hWnd)) {
        if (!hDC) {
            throw std::runtime_error("Failed to get window device context");
        }
    }

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


std::wstring toUTF16(const std::string& source) {
    int size = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, nullptr, 0);
    if (size == 0) {
        throw std::runtime_error("UTF-8 to UTF-16 conversion failed");
    }
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, &result[0], size);
    return result;
}


class SysWindow {
public:
    SysWindow() = default;
    
    SysWindow(const std::string& title, Position<int> position, Size<int> size)
        : wnd(
            NULL,
            getWndClass(),
            toUTF16(title).c_str(),
            WS_OVERLAPPEDWINDOW,
            position.x, position.y,
            size.width, size.height,
            NULL, NULL, getHInstance(), nullptr),
        dc(wnd)
    {}
    
    SysWindow(const SysWindow&) = delete;
    SysWindow& operator=(const SysWindow&) = delete;

    SysWindow(SysWindow&& other) noexcept
        : wnd(std::move(other.wnd)), dc(std::move(other.dc))
    {}

    SysWindow& operator=(SysWindow&& other) noexcept {
        wnd = std::move(other.wnd);
        dc = std::move(other.dc);
        return *this;
    }

    operator bool() const {
        return wnd && dc;
    }

    void show() {
        ShowWindow(wnd, SW_SHOW);
    }

private:

    static HINSTANCE getHInstance() {
        static HINSTANCE hInstance =
            reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
        return hInstance;
    }
    
    static WndClassHandler& getWndClass() {
        static WndClassHandler wc(
            CS_OWNDC,
            windowProc,
            getHInstance(),
            L"Istok");
        return wc;
    }

    WndHandler wnd;
    DCHandler dc;
};

/*
class Screen {
public:
    Screen() {}

    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

    Screen(Screen&& other) = delete;
    Screen& operator=(Screen&& other) = delete;

    void add(const std::string& id, std::unique_ptr<WindowWidget>&& window) {
        if (windows.contains(id)) {
            throw std::runtime_error("Window id is busy");
        }
        windows[id] = std::move(window);
        sysWindows[id] = std::make_unique<SysWindow>(
            id, Position<int>{100, 200}, Size<int>{400, 300});
        sysWindows[id]->show();
    }

private:
    std::map<std::string, std::unique_ptr<WindowWidget>> windows;
    std::map<std::string, std::unique_ptr<SysWindow>> sysWindows;
};


class Window: public WindowWidget {
public:
    void accept(WidgetVisitor& visitor) override {
        visitor.visit(*this, {});
    }
};
*/

int main() {
/*
    Screen screen;
    screen.add("main", std::make_unique<Window>());

    while (true) {
        MSG msg;
        GetMessageW(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
*/
    return 0;
}
