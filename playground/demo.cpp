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

struct Scene {
    Rect<float> texOut;
    Rect<float> texIn;
};

class WindowRenderer;

class Renderer {
public:    
    using NativeHandle = WinAPI::HWndWindow::NativeHandle;
    using Scene = Scene;
    using WindowRenderer = WindowRenderer;

    Renderer() {
        dummyWindow = std::make_unique<WinAPI::BasicWindow>(
            WindowParams{}, nullptr);
        HWND hWnd = dummyWindow->getHandle();
        WinAPI::prepareForGL(hWnd);
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
    
    std::unique_ptr<WindowRenderer> create();

    void prepare(NativeHandle handle) {
        WinAPI::prepareForGL(handle.hWnd);
    }

    class Scope {
    public:
        Scope(Renderer& renderer)
        : scope(
            renderer.gl,
            renderer.dummyWindow ? renderer.dummyWindow->getHandle() : nullptr
        ) {}
        
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

    ~Renderer() noexcept {
        try {
            WinAPI::WGL::Scope scope(gl, dummyWindow->getHandle());
            program.destroy(scope);
            texture.destroy(scope);
        } catch(...) {}
    }

private:
    std::unique_ptr<WinAPI::BasicWindow> dummyWindow;
    WinAPI::GLContext gl;
    OpenGL::ImageTexture<WinAPI::WGL> texture;
    OpenGL::ShaderProgram<WinAPI::WGL> program;
};


class WindowRenderer {
public:
    WindowRenderer() = default;

    using NativeHandle = WinAPI::HWndWindow::NativeHandle;
    using Scene = Scene;

    void loadScene(std::unique_ptr<Scene>&& scene) {
        this->scene = std::move(scene);
    }

    void prepare(NativeHandle handle) {
        master.prepare(handle);
        Renderer::Scope scope(master, handle);
        triangles = OpenGL::Triangle2DArray<WinAPI::WGL>(scope);
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
        triangles.clear();
        float mt = scene->texOut.top - scene->texIn.top;
        float mb = scene->texIn.bottom - scene->texOut.bottom;
        float ml = scene->texIn.left - scene->texOut.left;
        float mr = scene->texOut.right - scene->texIn.right;
        float wx = 512.0f / rect.right;
        float wy = 512.0f / rect.bottom;
        
        float wx0 = -1;
        float wx1 = wx0 + ml * wx;
        float wx3 = 1;
        float wx2 = wx3 - mr * wx;
        float wy0 = 1;
        float wy1 = wy0 - mt * wy;
        float wy3 = -1;
        float wy2 = wy3 + mb * wy;
        
        float tx0 = scene->texOut.left;
        float tx1 = scene->texIn.left;
        float tx2 = scene->texIn.right;
        float tx3 = scene->texOut.right;
        float ty0 = scene->texOut.top;
        float ty1 = scene->texIn.top;
        float ty2 = scene->texIn.bottom;
        float ty3 = scene->texOut.bottom;
        
        triangles.append(OpenGL::RectSprite(
            {wx0, wy0, wx1, wy1}, {tx0, ty0, tx1, ty1}, 0));
        triangles.append(OpenGL::RectSprite(
            {wx1, wy0, wx2, wy1}, {tx1, ty0, tx2, ty1}, 0));
        triangles.append(OpenGL::RectSprite(
            {wx2, wy0, wx3, wy1}, {tx2, ty0, tx3, ty1}, 0));
        triangles.append(OpenGL::RectSprite(
            {wx0, wy1, wx1, wy2}, {tx0, ty1, tx1, ty2}, 0));
        triangles.append(OpenGL::RectSprite(
            {wx1, wy1, wx2, wy2}, {tx1, ty1, tx2, ty2}, 0));
        triangles.append(OpenGL::RectSprite(
            {wx2, wy1, wx3, wy2}, {tx2, ty1, tx3, ty2}, 0));
        triangles.append(OpenGL::RectSprite(
            {wx0, wy2, wx1, wy3}, {tx0, ty2, tx1, ty3}, 0));
        triangles.append(OpenGL::RectSprite(
            {wx1, wy2, wx2, wy3}, {tx1, ty2, tx2, ty3}, 0));
        triangles.append(OpenGL::RectSprite(
            {wx2, wy2, wx3, wy3}, {tx2, ty2, tx3, ty3}, 0));
        triangles.draw(scope);
        scope.swapBuffers();
    }

    ~WindowRenderer() noexcept {
        try {
            Renderer::Scope scope(master);
            triangles.destroy(scope);
        } catch(...) {}
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

using Window = WinAPI::Window<WinAPI::HWndWindow, Renderer>;
using Platform = WinAPI::Platform<Entity, Window, Renderer>;
static_assert(GUIPlatform<Platform>);

int main() {
    std::cout << "main: start" << std::endl << std::flush;
    EntityComponentManager ecs;
    Renderer renderer;
    Platform gui;
    Entity window = ecs.createEntity();
    Entity menu = ecs.createEntity();
    gui.createWindow(window, WindowParams{{200, 100, 600, 400}, "Istok"});
    gui.createWindow(menu, WindowParams{{300, 200, 500, 500}, std::nullopt});
    float px = 1.0f / 256;
    gui.loadScene(window, std::make_unique<WindowRenderer::Scene>(
        Rect<float>{16 * px, 1 - 16 * px, 9 * 16 * px, 1 - 6 * 16 * px},
        Rect<float>{4 * 16 * px, 1 - (16 + 25) * px, 5 * 16 * px, 1 - (6 * 16 - 8) * px}
    ));
    gui.loadScene(menu, std::make_unique<WindowRenderer::Scene>(
        Rect<float>{16 * px, 1 - 16 * px, 9 * 16 * px, 1 - 6 * 16 * px},
        Rect<float>{4 * 16 * px, 1 - (16 + 25) * px, 5 * 16 * px, 1 - (6 * 16 - 8) * px}
    ));
    gui.setAreaTester(window, std::make_unique<Caption>(24));
    while (true) {
        PlatformEvent<Entity> msg = gui.getMessage();
        if (std::holds_alternative<PlatformEvents::Exception>(msg)) {
            try {
                std::rethrow_exception(
                    std::get<PlatformEvents::Exception>(msg).exception);
            } catch(const std::exception& ex) {
                std::cout << "main: gui exception: " << ex.what() << std::endl << std::flush;
            }
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
