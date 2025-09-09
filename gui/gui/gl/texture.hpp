// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <gui/common/platform.hpp>
#include "base.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <GL/glew.h>

#include <stdexcept>

namespace Istok::GUI::OpenGL {

template <typename GL>
class Texture2DHandle {
public:
    Texture2DHandle() = default;

    Texture2DHandle(GL::Scope& context) : owner(context) {
        glGenTextures(1, &handle);
        if (handle == 0) {
            throw std::runtime_error("Failed to generate OpenGL texture");
        }
    }

    operator bool() const noexcept {
        return handle != 0;
    }

    void destroy(GL::Scope& context) {
        ensureContext();
        safeDestroy();
    }

    ~Texture2DHandle() {
        safeDestroy();
    }

    Texture2DHandle(const Texture2DHandle&) = delete;
    Texture2DHandle& operator=(const Texture2DHandle&) = delete;
    
    Texture2DHandle(Texture2DHandle&& other)
    : handle(other.handle), owner(std::move(other.owner)) {
        other.drop();
    }
    
    Texture2DHandle& operator=(Texture2DHandle&& other) {
        if (&other == this) {
            return *this;
        }
        if (*this) {
            ensureContext();
            safeDestroy();
        }
        handle = other.handle;
        owner = std::move(other.owner);
        other.drop;
        return *this;
    }

    void bind(GL::Scope& context) {
        ensureContext();
        glBindTexture(GL_TEXTURE_2D, handle);
    }

private:
    GLuint handle = 0;
    GL::Owner owner;

    void safeDestroy() noexcept {
        if (*this && owner.isCurrent()) {
            glDeleteTextures(1, &handle);
            handle = 0;
        }
    }

    void ensureContext() {
        if (!owner.isCurrent()) {
            throw std::runtime_error("Out of owning context");
        }
    }

    void drop() {
        handle = 0;
    }
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


template <typename GL>
class ImageTexture {
public:
    ImageTexture(GL::Scope& context, const std::string& fileName)
    : texture(context) {
        Image img(fileName);
        if (img.getChannels() != 4) {
            throw std::runtime_error("Need 4 channels: " + fileName);
        }
        this->width = img.getWidth();
        this->height = img.getHeight();
        texture.bind(context);
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

    void destroy(GL::Scope& context) {
        texture.destroy(context);
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
