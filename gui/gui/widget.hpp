#pragma once

#include <string>
#include <vector>
#include <memory>


template <typename T>
using refvector = std::vector<std::reference_wrapper<T>>;

template <typename T>
using uptrvector = std::vector<std::unique_ptr<T>>;


class Widget;
class ImageWidget;
class TextWidget;
class CompositeWidget;
class WindowWidget;


class WidgetVisitor {
public:
    virtual void visit(ImageWidget& widget) = 0;
    virtual void visit(TextWidget& widget) = 0;
    virtual void visit(CompositeWidget& widget, refvector<Widget> children) = 0;
    virtual void visit(WindowWidget& widget, refvector<Widget> children) = 0;
};


class Widget {
public:
    virtual void accept(WidgetVisitor& visitor) = 0;
};


class ImageWidget: public Widget {
public:
    ImageWidget(const std::string& key) : key(key) {}

    const std::string& getKey() const {
        return key;
    }

    void accept(WidgetVisitor& visitor) {
        visitor.visit(*this);
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

    void accept(WidgetVisitor& visitor) {
        visitor.visit(*this);
    }

private:
    std::string text;
};


class CompositeWidget {};

class WindowWidget {};
