#include <ecs.hpp>
#include <gui/winapi/window.hpp>
#include <gui/winapi/wgl.hpp>
#include <gui/winapi/winapi.hpp>
#include <gui/gl/texture.hpp>
#include <gui/gl/shader.hpp>
#include <tools/queue.hpp>

#include <glm/glm.hpp>

#include <iostream>
#include <variant>
#include <vector>
#include <array>

using namespace Istok::GUI;
using namespace Istok::ECS;

template <typename T>
using RefVector = std::vector<std::reference_wrapper<T>>;


struct Vertex {
    glm::fvec2 pos;
    glm::fvec2 tex;
};

using Triangle = std::array<Vertex, 3>;

struct RectSprite {
    Triangle lb;
    Triangle rt;
    
    static RectSprite create(Rect<float> world, Rect<float> tex) {
        return {
            { {
                {{world.left, world.bottom}, {tex.left, tex.bottom}},
                {{world.right, world.bottom}, {tex.right, tex.bottom}},
                {{world.left, world.top}, {tex.left, tex.top}},
            } },
            { {
                {{world.right, world.top}, {tex.right, tex.top}},
                {{world.left, world.top}, {tex.left, tex.top}},
                {{world.right, world.bottom}, {tex.right, tex.bottom}},
            } }
        };
    }
};


class VertexArrayObject {
public:
    VertexArrayObject() {
        glGenVertexArrays(1, &id);
    }

    ~VertexArrayObject() {
        glDeleteVertexArrays(1, &id);
    }

    VertexArrayObject(const VertexArrayObject&) = delete;
    VertexArrayObject& operator=(const VertexArrayObject&) = delete;

    void bind() {
        glBindVertexArray(id);
    }

private:
    GLuint id;
};


class VertexBuffer {
public:
    VertexBuffer() {
        glGenBuffers(1, &id);
    }

    ~VertexBuffer() {
        glDeleteBuffers(1, &id);
    }

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    void bind() {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

private:
    GLuint id;
};


class VertexArray {
public:
    VertexArray() {
        vao.bind();
        vbo.bind();
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    }
    
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    int size() const { return data.size(); }

    void bind() {
        vao.bind();
        vbo.bind();
    }

    void clear() {
        data.clear();
    }

    void append(const Vertex& source) {
        data.push_back(source);
    }

    void append(const Triangle& source) {
        appendStruct(source);
    }

    void append(const RectSprite& source) {
        appendStruct(source);
    }

    void sync() {
        bind();
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * data.size(), data.data(), GL_STATIC_DRAW);
    }

private:
    VertexArrayObject vao;
    VertexBuffer vbo;
    std::vector<Vertex> data;

    template <typename T>
    void appendStruct(const T& source) {
        static_assert(sizeof(T) % sizeof(Vertex) == 0);
        constexpr int scale = sizeof(T) / sizeof(Vertex);
        int oldSize = data.size();
        data.resize(oldSize + scale);
        std::memcpy(
            data.data() + oldSize,
            &source,
            sizeof(T));
    }
};


class TriangleArray {
public:
    TriangleArray() {}
    TriangleArray(const TriangleArray&) = delete;
    TriangleArray& operator=(const TriangleArray&) = delete;

    int size() {
        return vertices.size() / 3;
    }

    void clear() {
        vertices.clear();
    }
    
    void append(const Triangle& source) {
        vertices.append(source);
    }

    void append(const RectSprite& source) {
        vertices.append(source);
    }
    
    void bind() {
        vertices.bind();
        if (outOfSync) {
            vertices.sync();
            outOfSync = false;
        }
    }

private:
    VertexArray vertices;
    bool outOfSync = true;
};


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
    
    private:
        WinAPI::WGL::Scope scope;
    };

private:
    WinAPI::GLContext gl;
    std::unique_ptr<OpenGL::ImageTexture<WinAPI::WGL>> texture;
    OpenGL::ShaderProgram<WinAPI::WGL> program;

    void init(HWND hWnd) {
        gl = WinAPI::GLContext(hWnd);
        WinAPI::WGL::Scope scope(gl, hWnd);
        texture = std::make_unique<OpenGL::ImageTexture<WinAPI::WGL>>(
            scope,
            "C:/Users/zholu/Documents/Programming/istok/playground/gui.png");
        OpenGL::Shader<WinAPI::WGL> vertex(scope, GL_VERTEX_SHADER, R"(
            #version 330 core
            
            layout (location = 0) in vec2 pos;
            layout (location = 1) in vec2 tex;
            out vec2 texCoord;

            void main() {
                gl_Position = vec4(pos, 0.0, 1.0);
                texCoord = tex;
            }
        )");
        OpenGL::Shader<WinAPI::WGL> fragment(scope, GL_FRAGMENT_SHADER, R"(
            #version 330 core
            in vec2 texCoord;
            out vec4 FragColor;

            uniform sampler2D atlas;

            void main() {
                FragColor = texture(atlas, texCoord);
            }
        )");
        std::vector<OpenGL::Shader<WinAPI::WGL>> shaders;
        shaders.push_back(std::move(vertex));
        shaders.push_back(std::move(fragment));
        program = OpenGL::ShaderProgram<WinAPI::WGL>(scope, shaders);
        program.use(scope);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture(GL_TEXTURE0);
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
        Renderer::Scope cl(master, handle);
        triangles = std::make_unique<TriangleArray>();
        float cell = 16.f / 256;
        triangles->append(RectSprite::create(
            {-1, 1, 1, -1},
            {cell, 1 - cell, 9 * cell, 6 * cell}));
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
        triangles->bind();
        glDrawArrays(GL_TRIANGLES, 0, triangles->size() * 3);
        scope.swapBuffers();
    }

private:
    Renderer& master;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<TriangleArray> triangles;

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
