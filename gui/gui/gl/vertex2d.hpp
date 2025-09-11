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
    glm::fvec2 pos;
    glm::fvec2 tex;
};

template <OpenGLContext GL>
class Vertex2DArray {
public:
    Vertex2DArray(GL::Scope& scope) {
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
    
    Vertex2DArray(const VertexArray&) = delete;
    Vertex2DArray& operator=(const VertexArray&) = delete;

    void bind(GL::Scope& scope) {
        vao.bind(scope);
        vbo.bind(scope);
    }

private:
    VertexArray vao;
    VertexBuffer vbo;
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

} // namespace Istok::GUI::OpenGL
