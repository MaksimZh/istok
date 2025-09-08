// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <gui/common/platform.hpp>
#include "base.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <GL/glew.h>

#include <stdexcept>

namespace Istok::GUI::GL {

class Texture2DHandle {
public:
    Texture2DHandle(CurrentGL& currentGL) {
        glGenTextures(1, &handle);
        if (handle == 0) {
            throw std::runtime_error("Failed to generate OpenGL texture");
        }
    }

    ~Texture2DHandle() {
        glDeleteTextures(1, &handle);
    }

    Texture2DHandle(const Texture2DHandle&) = delete;
    Texture2DHandle& operator=(const Texture2DHandle&) = delete;

    Texture2DHandle(Texture2DHandle&&) = delete;
    Texture2DHandle& operator=(Texture2DHandle&&) = delete;

    void bind() {
        glBindTexture(GL_TEXTURE_2D, handle);
    }

private:
    GLuint handle;
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


class ImageTexture {
public:
    ImageTexture(CurrentGL& currentGL, const std::string& fileName)
    : texture(currentGL) {
        Image img(fileName);
        if (img.getChannels() != 4) {
            throw std::runtime_error("Need 4 channels: " + fileName);
        }
        this->width = img.getWidth();
        this->height = img.getHeight();
        setup();
        glTexImage2D(
            GL_TEXTURE_2D,
            0, GL_RGBA,
            width, height,
            0, GL_RGBA, GL_UNSIGNED_BYTE,
            img.getData());
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
    Texture2DHandle texture;
    int width;
    int height;

    void setup() {
        texture.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
};


} // namespace Istok::GUI::GL
