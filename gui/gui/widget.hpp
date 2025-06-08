#pragma once

#include <string>
#include <vector>


template <typename T>
using refvector = std::vector<std::reference_wrapper<T>>;


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


class Widget {};


class ImageWidget {
public:
    ImageWidget(const std::string& id) : id(id) {}

    const std::string& getId() const {
        return id;
    }

    void accept(WidgetVisitor& visitor) {
        visitor.visit(*this);
    }

private:
    std::string id;
};


class TextWidget {
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
