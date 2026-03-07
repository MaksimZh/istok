// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "base.hpp"

#include <string>
#include <vector>

namespace Istok::GUI::OpenGL {

template <OpenGLContext GL>
class Shader {
public:
    Shader() = default;
    
    Shader(GL::Scope& scope, GLenum type, const std::string& source)
    : handle(scope, glCreateShader(type)) {
        if (!handle) {
            throw std::runtime_error("Failed to create shader");
        }

        GLuint h = handle.get(scope);

        const char* codePtr = source.c_str();
        glShaderSource(h, 1, &codePtr, nullptr);
        glCompileShader(h);

        GLint success;
        glGetShaderiv(h, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(h, sizeof(infoLog), nullptr, infoLog);
            throw std::runtime_error(
                "Shader compilation failed: " + std::string(infoLog));
        }
    }

    void destroy(GL::Scope& scope) {
        handle.destroy(scope);
    }

    GLuint get(GL::Scope& scope) const {
        return handle.get(scope);
    }

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) = default;
    Shader& operator=(Shader&& other) = default;

private:
    struct Deleter {
        static void destroy(GLuint handle) {
            glDeleteShader(handle);
        }
    };

    ResourceHandle<GL, Deleter> handle;
};


template <OpenGLContext GL>
class ShaderProgram {
public:
    ShaderProgram() = default;
    
    ShaderProgram(GL::Scope& scope, const std::vector<Shader<GL>>& shaders)
    : handle(scope, glCreateProgram()) {
        if (!handle) {
            throw std::runtime_error("Failed to create shader program");
        }
        GLuint h = handle.get(scope);
        for (auto& s : shaders) {
            glAttachShader(h, s.get(scope));
        }
        glLinkProgram(h);

        GLint success;
        glGetProgramiv(h, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(h, sizeof(infoLog), nullptr, infoLog);
            throw std::runtime_error(
                "Shader program linking failed: " + std::string(infoLog));
        }

        for (auto& s : shaders) {
            glDetachShader(h, s.get(scope));
        }
    }
    
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&& other) = default;
    ShaderProgram& operator=(ShaderProgram&& other) = default;

    void destroy(GL::Scope& scope) {
        handle.destroy(scope);
    }

    void use(GL::Scope& scope) {
        glUseProgram(handle.get(scope));
    }

    GLuint getUniformLocation(GL::Scope& scope, std::string name) {
        return glGetUniformLocation(handle.get(scope), name.c_str());
    }

private:
    struct Deleter {
        static void destroy(GLuint handle) {
            glDeleteProgram(handle);
        }
    };

    ResourceHandle<GL, Deleter> handle;
};


} // namespace Istok::GUI::OpenGL
