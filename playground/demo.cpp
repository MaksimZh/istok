#include <ecs.hpp>
#include <gui/winapi/window.hpp>
#include <gui/winapi/wgl.hpp>
#include <gui/winapi/winapi.hpp>
#include <gui/gl/texture.hpp>
#include <gui/gl/shader.hpp>
#include <gui/gl/vertex2d.hpp>
#include <tools/queue.hpp>

#include <glm/glm.hpp>

#include <iostream>
#include <variant>
#include <vector>
#include <array>

using namespace Istok::GUI;
using namespace Istok::ECS;


class WindowRenderer;

class Renderer {
public:
    using NativeHandle = WinAPI::HWndWindow::NativeHandle;
    
    std::unique_ptr<WindowRenderer> create();

    void prepare(NativeHandle handle) {
        WinAPI::prepareForGL(handle.hWnd);
        if (!gl) {
            init(handle.hWnd);
        }
    }

    class Scope {
    public:
        Scope(Renderer& renderer, NativeHandle handle)
        : scope(renderer.gl, handle.hWnd) {}

        void swapBuffers() {
            scope.swapBuffers();
        }

        operator WinAPI::WGL::Scope&() {
            return scope;
        }
    
    private:
        WinAPI::WGL::Scope scope;
    };

private:
    WinAPI::GLContext gl;
    OpenGL::ImageTexture<WinAPI::WGL> texture;
    OpenGL::ShaderProgram<WinAPI::WGL> program;

    void init(HWND hWnd) {
        gl = WinAPI::GLContext(hWnd);
        WinAPI::WGL::Scope scope(gl, hWnd);
        texture = OpenGL::ImageTexture<WinAPI::WGL>(
            scope,
            "C:/Users/zholu/Documents/Programming/istok/playground/gui.png");
        OpenGL::Shader<WinAPI::WGL> vertex(scope, GL_VERTEX_SHADER, R"(
            #version 330 core
            
            layout (location = 0) in vec2 pos;
            layout (location = 1) in vec2 tex;
            layout (location = 2) in float pal;
            out vec2 texCoord;
            out float palIndex;

            void main() {
                gl_Position = vec4(pos, 0.0, 1.0);
                texCoord = tex;
                palIndex = pal;
            }
        )");
        OpenGL::Shader<WinAPI::WGL> fragment(scope, GL_FRAGMENT_SHADER, R"(
            #version 330 core
            in vec2 texCoord;
            in float palIndex;
            out vec4 FragColor;

            uniform sampler2D atlas;
            uniform vec2 palOrigin;
            uniform float palStride;
            uniform vec2 palRGWindow;
            uniform float palBStride;

            void main() {
                vec4 t = texture(atlas, texCoord);
                vec2 uv = palOrigin
                    + vec2(palIndex * palStride, 0)
                    + t.rg * palRGWindow;
                vec4 p0 = texture(atlas, uv);
                vec4 p1 = texture(atlas, uv + vec2(0, palBStride));
                vec4 p = mix(p0, p1, t.b);
                FragColor = vec4(mix(t.rgb, p.rgb, p.a), t.a);
            }
        )");
        std::vector<OpenGL::Shader<WinAPI::WGL>> shaders;
        shaders.push_back(std::move(vertex));
        shaders.push_back(std::move(fragment));
        program = OpenGL::ShaderProgram<WinAPI::WGL>(scope, shaders);
        program.use(scope);
        float u = 16.f / 256; // convenience unit of measurement (16 pixels)
        float h = 0.5f / 256; // half pixel
        glUniform2f(program.getUniformLocation(scope, "palOrigin"), u + h, u + h);
        glUniform1f(program.getUniformLocation(scope, "palStride"), u);
        glUniform2f(program.getUniformLocation(scope, "palRGWindow"), u - 2 * h, u - 2 * h);
        glUniform1f(program.getUniformLocation(scope, "palBStride"), u);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
};


class WindowRenderer {
public:
    WindowRenderer() = default;

    using NativeHandle = WinAPI::HWndWindow::NativeHandle;
    
    struct Scene {
        float r;
        float g;
        float b;
        float a;
    };

    void loadScene(std::unique_ptr<Scene>&& scene) {
        this->scene = std::move(scene);
    }

    void prepare(NativeHandle handle) {
        master.prepare(handle);
        Renderer::Scope scope(master, handle);
        triangles = OpenGL::Triangle2DArray<WinAPI::WGL>(scope);
        float cell = 16.f / 256;
        triangles.append(OpenGL::RectSprite(
            {-1, 1, 1, -1},
            {cell, 1 - cell, 9 * cell, 1 - 6 * cell},
            5));
    }

    void draw(NativeHandle handle) {
        if (scene == nullptr) {
            return;
        }
        Renderer::Scope scope(master, handle);
        RECT rect;
        GetClientRect(handle.hWnd, &rect);
        glViewport(0, 0, rect.right, rect.bottom);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        triangles.draw(scope);
        scope.swapBuffers();
    }

private:
    Renderer& master;
    std::unique_ptr<Scene> scene;
    OpenGL::Triangle2DArray<WinAPI::WGL> triangles;

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
