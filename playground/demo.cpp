#include <iostream>
#include <memory>
#include <windows.h>


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


int main() {
    HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
        
    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = windowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Istok";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    LPCWSTR id = MAKEINTATOM(RegisterClass(&wc));
    if (!id) {
        throw runtime_error("Failed to register window class");
    }

    HWND hWnd = CreateWindowEx(
        0,
        id,
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
