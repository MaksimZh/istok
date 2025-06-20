#pragma once

#include <vector>
#include <span>
#include <memory>


class Widget;

class WidgetHandler {
public:
    WidgetHandler(Widget& widget) : widget(widget) {}

    Widget& getWidget() {
        return widget;
    }

private:
    Widget& widget;
};


class Widget {
public:
    Widget* getBase() {
        return base;
    }
    
    void setBase(Widget* widget) {
        base = widget;
    }

    int numParts() const {
        return parts.size();
    }

    std::span<Widget* const> getParts() {
        return std::span<Widget* const>(parts.data(), parts.size());
    }

    void addPart(Widget* part) {
        parts.push_back(part);
    }

    WidgetHandler* getHandler() {
        return handler.get();
    }

    template <typename H>
    void createHandler() {
        handler = std::make_unique<H>(*this);
    }


private:
    Widget* base = nullptr;
    std::vector<Widget*> parts;
    std::unique_ptr<WidgetHandler> handler;
};
