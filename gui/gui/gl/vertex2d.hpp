// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <gui/common/platform.hpp>
#include "base.hpp"
#include "buffer.hpp"

#include <glm/glm.hpp>

#include <array>
#include <vector>

namespace Istok::GUI::OpenGL {

struct Vertex2D {
    glm::fvec2 xy;
    glm::fvec2 uv;
    //glm::fvec4 rgba;
};


template <OpenGLContext GL>
class Vertex2DArray {
public:
    Vertex2DArray() = default;
    
    Vertex2DArray(GL::Scope& scope)
    : vao(scope), vbo(scope) {
        vao.bind(scope);
        vbo.bind(scope);
        
        glVertexAttribPointer(
            0, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    }
    
    Vertex2DArray(const Vertex2DArray&) = delete;
    Vertex2DArray& operator=(const Vertex2DArray&) = delete;
    Vertex2DArray(Vertex2DArray&&) = default;
    Vertex2DArray& operator=(Vertex2DArray&&) = default;

    void bind(GL::Scope& scope) {
        vao.bind(scope);
        vbo.bind(scope);
    }

private:
    VertexArray<GL> vao;
    VertexBuffer<GL> vbo;
};


using Triangle2D = std::array<Vertex2D, 3>;


struct RectSprite {
    Triangle2D lb;
    Triangle2D rt;
    
    RectSprite(Rect<float> world, Rect<float> tex)
    :   lb({
            {{world.left, world.bottom}, {tex.left, tex.bottom}},
            {{world.right, world.bottom}, {tex.right, tex.bottom}},
            {{world.left, world.top}, {tex.left, tex.top}}
        }),
        rt({
            {{world.right, world.top}, {tex.right, tex.top}},
            {{world.left, world.top}, {tex.left, tex.top}},
            {{world.right, world.bottom}, {tex.right, tex.bottom}},
        }) {}
};


template <OpenGLContext GL>
class Triangle2DArray {
public:
    Triangle2DArray() = default;
    
    Triangle2DArray(GL::Scope& scope) : vertices(scope) {}
    
    Triangle2DArray(const Triangle2DArray&) = delete;
    Triangle2DArray& operator=(const Triangle2DArray&) = delete;
    Triangle2DArray(Triangle2DArray&&) = default;
    Triangle2DArray& operator=(Triangle2DArray&&) = default;
    
    void append(const Triangle2D& source) {
        appendStruct(source);
        ready = false;
    }

    void append(const RectSprite& source) {
        appendStruct(source);
        ready = false;
    }
    
    void draw(GL::Scope& scope) {
        vertices.bind(scope);
        if (!ready) {
            glBufferData(
                GL_ARRAY_BUFFER,
                sizeof(Triangle2D) * data.size(),
                data.data(),
                GL_STATIC_DRAW);
            ready = true;
        }
        glDrawArrays(GL_TRIANGLES, 0, data.size() * 3);
    }

private:
    Vertex2DArray<GL> vertices;
    std::vector<Triangle2D> data;
    bool ready = true;

    template <typename T>
    void appendStruct(const T& source) {
        static_assert(sizeof(T) % sizeof(Triangle2D) == 0);
        constexpr int scale = sizeof(T) / sizeof(Triangle2D);
        int oldSize = data.size();
        data.resize(oldSize + scale);
        std::memcpy(
            data.data() + oldSize,
            &source,
            sizeof(T));
    }
};


} // namespace Istok::GUI::OpenGL
