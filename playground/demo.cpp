#include <string>
#include <optional>
#include <memory>
#include <map>

#include <gui/core/tools.hpp>
#include <gui/core/widget.hpp>
#include <gui/winapi/window.hpp>


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
