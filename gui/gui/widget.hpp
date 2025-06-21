#pragma once

#include <vector>
#include <span>
#include <memory>
#include <ranges>


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

    int numParts() const {
        return parts.size();
    }

    std::span<Widget* const> getParts() {
        return std::span<Widget* const>(parts.data(), parts.size());
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

    friend void attach(Widget&, Widget&);
    friend void detach(Widget&);
};


void attach(Widget& base, Widget& part) {
    assert(
        std::ranges::find(base.parts.begin(), base.parts.end(), &part)
        == base.parts.end());
    assert(part.base == nullptr);
    base.parts.push_back(&part);
    part.base = &base;
}


void detach(Widget& part) {
    assert(part.base != nullptr);
    Widget& base = *(part.base);
    auto pos = std::ranges::find(base.parts.begin(), base.parts.end(), &part);
    assert(pos != base.parts.end());
    base.parts.erase(pos);
    part.base = nullptr;
}
