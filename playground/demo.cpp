#include <ecs.hpp>
#include <gui/winapi/window.hpp>
#include <gui/winapi/gl.hpp>
#include <gui/winapi/winapi.hpp>
#include <tools/queue.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>

#include <iostream>
#include <variant>
#include <vector>
#include <array>

using namespace Istok::GUI;
using namespace Istok::ECS;

template <typename T>
using RefVector = std::vector<std::reference_wrapper<T>>;

class Texture2DBase {
public:
    Texture2DBase() {
        glGenTextures(1, &id);
        if (id == 0) {
            throw std::runtime_error("Failed to generate OpenGL texture");
        }
    }

    ~Texture2DBase() {
        glDeleteTextures(1, &id);
    }

    Texture2DBase(const Texture2DBase&) = delete;
    Texture2DBase& operator=(const Texture2DBase&) = delete;

    void bind() {
        glBindTexture(GL_TEXTURE_2D, id);
    }

    GLuint getId() {
        return id;
    }

private:
    GLuint id;
};


class Image {
public:
    Image(const std::string& fileName) {
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load(fileName.c_str(), &width, &height, &channels, 0);
        if (!data) {
            throw std::runtime_error("Failed to load: " + fileName);
        }
    }

    ~Image() {
        stbi_image_free(data);
    }

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    unsigned char* getData() {
        return data;
    }

    int getWidth() {
        return width;
    }

    int getHeight() {
        return height;
    }

    int getChannels() {
        return channels;
    }

private:
    unsigned char* data;
    int width;
    int height;
    int channels;
};


class ImageTexture : public Texture2DBase {
public:
    ImageTexture(int width, int height) {
        this->width = width;
        this->height = height;
        setup();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    ImageTexture(const std::string& fileName) {
        Image img(fileName);
        if (img.getChannels() != 4) {
            throw std::runtime_error("Need 4 channels: " + fileName);
        }
        this->width = img.getWidth();
        this->height = img.getHeight();
        setup();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getData());
    }

    int getWidth() {
        return width;
    }

    int getHeight() {
        return height;
    }

    Rect<float> px2uv(Rect<int> src) {
        return {
            (float)src.left / width,
            (float)src.bottom / height,
            (float)src.right / width,
            (float)src.top / height,
        };
    }

private:
    int width;
    int height;

    void setup() {
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
};


class ShaderHandler {
public:
    ShaderHandler(GLuint value = 0) {
        id = value;
    }

    ~ShaderHandler() {
        if (id) {
            glDeleteShader(id);
        }
    }

    ShaderHandler(const ShaderHandler&) = delete;
    ShaderHandler& operator=(const ShaderHandler&) = delete;

    ShaderHandler& operator=(GLuint value) {
        if (id) {
            throw std::runtime_error("Handler rewrite is forbidden");
        }
        id = value;
        return *this;
    }

    operator GLuint() const { return id; }
    explicit operator bool() const { return id != 0; }

private:
    GLuint id;
};


class Shader {
public:
    Shader(GLenum type, const std::string& source) {
        id = glCreateShader(type);
        if (!id) {
            throw std::runtime_error("Failed to create shader");
        }
        
        const char* codePtr = source.c_str();
        glShaderSource(id, 1, &codePtr, nullptr);
        glCompileShader(id);

        GLint success;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(id, sizeof(infoLog), nullptr, infoLog);
            throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
        }
    }

    GLuint getId() const { return id; }

private:
    ShaderHandler id;
};


class ShaderProgram {
public:
    ShaderProgram() {
        id = glCreateProgram();
        if (id == 0) {
            throw std::runtime_error("Failed to create shader");
        }
    }

    ~ShaderProgram() {
        glDeleteProgram(id);
    }

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    void link(RefVector<Shader> shaders) {
        for (auto& ref : shaders) {
            glAttachShader(id, ref.get().getId());
        }
        glLinkProgram(id);

        GLint success;
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
            throw std::runtime_error("Shader program linking failed: " + std::string(infoLog));
        }

        for (auto& ref : shaders) {
            glDetachShader(id, ref.get().getId());
        }
    }

    GLuint getId() const { return id; }

    void use() {
        glUseProgram(id);
    }

    GLuint getUniformLocation(std::string name) {
        return glGetUniformLocation(id, name.c_str());
    }

private:
    GLuint id;
};


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


class Program : public ShaderProgram {
public:
    Program() {
        Shader vertex(GL_VERTEX_SHADER, R"(
            #version 330 core
            
            layout (location = 0) in vec2 pos;
            layout (location = 1) in vec2 tex;
            out vec2 texCoord;

            void main() {
                gl_Position = vec4(pos, 0.0, 1.0);
                texCoord = tex;
            }
        )");
        Shader fragment(GL_FRAGMENT_SHADER, R"(
            #version 330 core
            in vec2 texCoord;
            out vec4 FragColor;

            uniform sampler2D atlas;

            void main() {
                FragColor = texture(atlas, texCoord);
            }
        )");
        RefVector<Shader> shaders = { vertex, fragment };
        link(shaders);
        use();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture(GL_TEXTURE0);
    }
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

    class ContextLock {
    public:
        ContextLock(Renderer& renderer, NativeHandle handle)
        : context(renderer.gl, handle.hWnd) {}
    private:
        WinAPI::CurrentGL context;
    };

private:
    WinAPI::GLContext gl;
    std::unique_ptr<ImageTexture> texture;
    std::unique_ptr<Program> program;

    void init(HWND hWnd) {
        gl = WinAPI::GLContext(hWnd);
        WinAPI::CurrentGL context(gl, hWnd);
        texture = std::make_unique<ImageTexture>(
            "C:/Users/zholu/Documents/Programming/istok/playground/gui.png");
        program = std::make_unique<Program>();
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
        Renderer::ContextLock cl(master, handle);
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
        Renderer::ContextLock cl(master, handle);
        RECT rect;
        GetClientRect(handle.hWnd, &rect);
        glViewport(0, 0, rect.right, rect.bottom);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        triangles->bind();
        glDrawArrays(GL_TRIANGLES, 0, triangles->size() * 3);
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
