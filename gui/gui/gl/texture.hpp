// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <gui/common/platform.hpp>
#include "base.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <GL/glew.h>

#include <stdexcept>

namespace Istok::GUI::OpenGL {

template <OpenGLContext GL>
class Texture2DHandle {
public:
    Texture2DHandle() = default;

    Texture2DHandle(GL::Scope& scope) {
        GLuint value;
        glGenTextures(1, &value);
        if (!value) {
            throw std::runtime_error("Failed to generate OpenGL texture");
        }
        handle = Handle(scope, value);
    }

    void destroy(GL::Scope& scope) {
        handle.destroy(scope);
    }

    Texture2DHandle(const Texture2DHandle&) = delete;
    Texture2DHandle& operator=(const Texture2DHandle&) = delete;
    Texture2DHandle(Texture2DHandle&& other) = default;
    Texture2DHandle& operator=(Texture2DHandle&& other) = default;

    void bind(GL::Scope& scope) {
        glBindTexture(GL_TEXTURE_2D, handle.get(scope));
    }

private:
    struct Deleter {
        static void destroy(GLuint handle) {
            glDeleteTextures(1, &handle);
        }
    };

    using Handle = ResourceHandle<GL, Deleter>;
    Handle handle;
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


template <OpenGLContext GL>
class ImageTexture {
public:
    ImageTexture(GL::Scope& scope, const std::string& fileName)
    : texture(scope) {
        Image img(fileName);
        if (img.getChannels() != 4) {
            throw std::runtime_error("Need 4 channels: " + fileName);
        }
        this->width = img.getWidth();
        this->height = img.getHeight();
        texture.bind(scope);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(
            GL_TEXTURE_2D,
            0, GL_RGBA,
            width, height,
            0, GL_RGBA, GL_UNSIGNED_BYTE,
            img.getData());
    }

    ImageTexture(const ImageTexture&) = delete;
    ImageTexture& operator=(const ImageTexture&) = delete;
    ImageTexture(ImageTexture&&) = default;
    ImageTexture& operator=(ImageTexture&&) = default;

    void destroy(GL::Scope& scope) {
        texture.destroy(scope);
    }

    int getWidth() {
        return width;
    }

    int getHeight() {
        return height;
    }

private:
    Texture2DHandle<GL> texture;
    int width;
    int height;
};


} // namespace Istok::GUI::OpenGL
