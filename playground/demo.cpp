#include <ecs.hpp>
#include <gui/winapi/window.hpp>
#include <gui/winapi/gl.hpp>
#include <gui/winapi/winapi.hpp>
#include <tools/queue.hpp>

#include <iostream>
#include <variant>

using namespace Istok::GUI;
using namespace Istok::ECS;

class WindowRenderer;

class Renderer {
public:
    std::unique_ptr<WindowRenderer> create();

    void prepare(WinAPI::HWndWindow& window) {
        WinAPI::prepareForGL(window);
        if (!gl) {
            gl = WinAPI::GLContext(window.sysContext().hWnd);
        }
    }

    class ContextLock {
    public:
        ContextLock(Renderer& renderer, WinAPI::HWndWindow& window)
        : context(renderer.gl, window) {}
    private:
        WinAPI::CurrentGL context;
    };

private:
    WinAPI::GLContext gl;
};


class WindowRenderer {
public:
    WindowRenderer() = default;
    
    struct Scene {
        float r;
        float g;
        float b;
        float a;
    };

    void loadScene(std::unique_ptr<Scene>&& scene) {
        this->scene = std::move(scene);
    }

    void prepare(WinAPI::HWndWindow& window) {
        master.prepare(window);
    }

    void draw(WinAPI::HWndWindow& window) {
        if (scene == nullptr) {
            return;
        }
        Renderer::ContextLock cl(master, window);
        glClearColor(scene->r, scene->g, scene->b, scene->a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

private:
    Renderer& master;
    std::unique_ptr<Scene> scene;

    friend Renderer;

    WindowRenderer(Renderer& master) : master(master) {}
};


std::unique_ptr<WindowRenderer> Renderer::create() {
    return std::unique_ptr<WindowRenderer>(new WindowRenderer(*this));
}


struct Caption: public WindowAreaTester {
    int height;

    Caption(int height) : height(height) {}
    
    WindowArea testWindowArea(Position<int> position) const noexcept override {
        return (position.y < height)
            ? WindowArea::moving
            : WindowArea::client;
    }
};

using Window = WinAPI::Window<WinAPI::HWndWindow, WindowRenderer>;
using Platform = WinAPI::Platform<Entity, Window>;
static_assert(GUIPlatform<Platform>);

int main() {
    std::cout << "main: start" << std::endl << std::flush;
    EntityComponentManager ecs;
    Renderer renderer;
    Platform gui;
    Entity window = ecs.createEntity();
    Entity menu = ecs.createEntity();
    gui.createWindow(window, WindowParams{{200, 100, 600, 400}, "Istok"});
    gui.createWindow(menu, WindowParams{{300, 200, 400, 500}, std::nullopt});
    gui.setRenderer(window, renderer.create());
    gui.setRenderer(menu, renderer.create());
    gui.loadScene(window, std::make_unique<WindowRenderer::Scene>(0.f, 1.f, 0.f, 0.f));
    gui.loadScene(menu, std::make_unique<WindowRenderer::Scene>(0.f, 0.f, 1.f, 0.f));
    gui.setAreaTester(window, std::make_unique<Caption>(32));
    while (true) {
        PlatformEvent<Entity> msg = gui.getMessage();
        if (std::holds_alternative<PlatformEvents::Exception>(msg)) {
            std::cout << "main: gui exception" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<PlatformEvents::Shutdown>(msg)) {
            std::cout << "main: gui shutdown" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<PlatformEvents::WindowClose<Entity>>(msg)) {
            std::cout << "main: closed" << std::endl << std::flush;
            auto ent = std::get<PlatformEvents::WindowClose<Entity>>(msg).id;
            gui.destroyWindow(ent);
            if (ent == window) {
                break;
            }
        }
    }
    std::cout << "main: end" << std::endl << std::flush;
    return 0;
}
