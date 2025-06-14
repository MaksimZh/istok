#pragma once

#include "visitor.hpp"

#include <string>
#include <vector>
#include <memory>
#include <utility>


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


class CompositeWidget: public Widget {/*
public:
    Position<float> getChildPosition(const std::string& id) const {
        return childPositions.at(id);
    }

protected:
    void setChildPosition(const std::string& id, Position<float> value) {
        childPositions[id] = value;
    }

private:
    std::map<const std::string&, Position<float>> childPositions;
*/};

class WindowWidget: public CompositeWidget {};
