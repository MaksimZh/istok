#include <ecs.hpp>
#include <gui/platform/winapi.hpp>
#include <tools/queue.hpp>

#include <iostream>
#include <variant>

using namespace Istok::GUI;
using namespace Istok::ECS;


class Renderer {
public:
    Renderer() = default;
    
    struct Scene {
        float r;
        float g;
        float b;
        float a;
    };


    class WindowRendererCore {
    public:
        WindowRendererCore(
            Renderer& renderer,
            WinAPI::SysWindow& window
        ) : renderer(renderer), window(window) {
            WinAPI::prepareForGL(window);
            if (!renderer.gl) {
                renderer.gl = WinAPI::GLContext(window.sysContext().hWnd);
            }
        }

        class CurrentWindowGL {
        public:
            CurrentWindowGL(WindowRendererCore core)
                : gl(core.renderer.gl, core.window) {}
        private:
            WinAPI::CurrentGL gl;
        };

    private:
        Renderer& renderer;
        WinAPI::SysWindow& window;
    };


    class WindowRenderer {
    public:
        WindowRenderer(
            Renderer& renderer,
            WinAPI::SysWindow& window
        ) : core(renderer, window) {}
        
        void loadScene(std::unique_ptr<Scene>&& scene) {
            this->scene = std::move(scene);
        }

        void draw() {
            if (scene == nullptr) {
                return;
            }
            WindowRendererCore::CurrentWindowGL gl(core);
            glClearColor(scene->r, scene->g, scene->b, scene->a);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    
    private:
        WindowRendererCore core;
        std::unique_ptr<Scene> scene;
    };

private:
    WinAPI::GLContext gl;
};


int main() {
    std::cout << "main: start" << std::endl << std::flush;
    EntityComponentManager ecs;
    Renderer renderer;
    WinAPI::Platform<Entity, WinAPI::SysWindow, Renderer> gui(renderer);
    Entity window = ecs.createEntity();
    Entity menu = ecs.createEntity();
    gui.createWindow(window, WindowParams{{200, 100, 600, 400}, "Istok"});
    gui.createWindow(menu, WindowParams{{300, 200, 400, 500}, std::nullopt});
    gui.loadScene(window, std::make_unique<Renderer::Scene>(0.f, 1.f, 0.f, 0.f));
    gui.loadScene(menu, std::make_unique<Renderer::Scene>(0.f, 0.f, 1.f, 0.f));
    while (true) {
        PlatformEvent<Entity> msg = gui.getMessage();
        if (std::holds_alternative<Event::PlatformHeartbeatTimeout>(msg)) {
            std::cout << "main: gui timeout" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<Event::PlatformException>(msg)) {
            std::cout << "main: gui exception" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<Event::PlatformShutdown>(msg)) {
            std::cout << "main: gui shutdown" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<Event::WindowClose<Entity>>(msg)) {
            std::cout << "main: closed" << std::endl << std::flush;
            auto ent = std::get<Event::WindowClose<Entity>>(msg).id;
            gui.destroyWindow(ent);
            if (ent == window) {
                break;
            }
        }
    }
    std::cout << "main: end" << std::endl << std::flush;
    return 0;
}
