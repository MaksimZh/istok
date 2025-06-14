#pragma once

#include "visitor.hpp"

#include <string>
#include <vector>
#include <memory>
#include <utility>


template <typename T>
using refvector = std::vector<std::reference_wrapper<T>>;


template <typename T>
struct Position {
    T x;
    T y;
};


template <typename T>
struct Size {
    T width;
    T height;
};


class Widget {
public:
    virtual void accept(Visitor<Widget>& visitor) {
        visitor.visit(*this);
    }

    Size<float> getSize() const {
        return size;
    }

    void setSize(Size<float> value) {
        size = value;
    }

private:
    Size<float> size;
};


class ImageWidget: public Widget {
public:
    ImageWidget(const std::string& key) : key(key) {}

    const std::string& getKey() const {
        return key;
    }

private:
    std::string key;
};


class TextWidget: public Widget {
public:
    TextWidget(const std::string& text) : text(text) {}

    const std::string& getText() const {
        return text;
    }

private:
    std::string text;
};


class CompositeWidget: public Widget {
public:
    virtual refvector<Widget> getChildren() = 0;
    virtual Position<float> getPosition(Widget& child) = 0;
    virtual void measure() = 0;
    virtual void arrange() = 0;
};

class WindowWidget: public CompositeWidget {};
