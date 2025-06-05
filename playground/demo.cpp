#include <iostream>
#include <memory>
#include <windows.h>
#include <optional>


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

template <typename T>
class Handler {
public:
    Handler() : val(nullopt) {}
    Handler(T src) : val(src) {}

    Handler(const Handler&) = delete;
    Handler& operator=(const Handler&) = delete;

    Handler& operator=(T src) {
        if (val.has_value()) {
            throw runtime_error("Handler rewrite is forbidden");
        }
        val = src;
        return *this;
    }

    bool has_value() const {
        return val.has_value();
    }

    T value() {
        return val.value();
    }

private:
    optional<T> val;
};


class WCHandler : public Handler<LPCWSTR> {
public:
    WCHandler(HINSTANCE hInstance) : hInstance(hInstance) {}
    WCHandler(LPCWSTR src, HINSTANCE hInstance)
        : Handler(src), hInstance(hInstance) {}
    using Handler::operator=;

    ~WCHandler() {
        if (!has_value() || !value()) {
            return;
        }
        UnregisterClass(value(), hInstance);
    }

private:
    HINSTANCE hInstance;
};


class HWNDHandler : public Handler<HWND> {
public:
    HWNDHandler(HWND src) : Handler(src) {}
    using Handler::operator=;

    ~HWNDHandler() {
        if (!has_value() || !value()) {
            return;
        }
        DestroyWindow(value());
    }
};


int main() {
    HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
        
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = windowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Istok";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    WCHandler windowClass(MAKEINTATOM(RegisterClassEx(&wc)), hInstance);
    if (!windowClass.value()) {
        throw runtime_error("Failed to register window class");
    }

    HWND hWnd = CreateWindowEx(
        0,
        windowClass.value(),
        L"Istok Demo",
        WS_OVERLAPPEDWINDOW,
        100, 200, 400, 300,
        NULL,
        NULL,
        hInstance,
        nullptr
    );

    if (!hWnd) {
        throw runtime_error("Failed to create window");
    }

    ShowWindow(hWnd, SW_SHOW);
    
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
