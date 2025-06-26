// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/widget.hpp>


#include <vector>
#include <algorithm>


namespace {

    class FakeWidget: public Widget {
    public:
        std::vector<Widget*> children;

        void addChild(Widget& widget) {
            attach(widget);
            children.push_back(&widget);
        }

        void removeChild(Widget& widget) {
            auto pos = std::find(children.begin(), children.end(), &widget);
            children.erase(pos);
            detach(widget);
        }
    };

}


TEST_CASE("Widget - size", "[unit][gui]") {
    FakeWidget w;
    w.setSize({10, 20});
    auto size = w.getSize();
    REQUIRE_THAT(size.width, Catch::Matchers::WithinAbs(10.0, 1e-9));
    REQUIRE_THAT(size.height, Catch::Matchers::WithinAbs(20.0, 1e-9));
}


TEST_CASE("Widget - tree", "[unit][gui]") {
    FakeWidget a, b, c, d, e;
    REQUIRE(a.getParent() == nullptr);
    REQUIRE(b.getParent() == nullptr);
    REQUIRE(c.getParent() == nullptr);
    REQUIRE(d.getParent() == nullptr);
    REQUIRE(e.getParent() == nullptr);
    a.addChild(b);
    c.addChild(d);
    REQUIRE(a.getParent() == nullptr);
    REQUIRE(b.getParent() == &a);
    REQUIRE(c.getParent() == nullptr);
    REQUIRE(d.getParent() == &c);
    REQUIRE(e.getParent() == nullptr);
    a.addChild(c);
    c.addChild(e);
    REQUIRE(a.getParent() == nullptr);
    REQUIRE(b.getParent() == &a);
    REQUIRE(c.getParent() == &a);
    REQUIRE(d.getParent() == &c);
    REQUIRE(e.getParent() == &c);
    a.removeChild(c);
    c.removeChild(e);
    REQUIRE(a.getParent() == nullptr);
    REQUIRE(b.getParent() == &a);
    REQUIRE(c.getParent() == nullptr);
    REQUIRE(d.getParent() == &c);
    REQUIRE(e.getParent() == nullptr);
    a.removeChild(b);
    c.removeChild(d);
    REQUIRE(a.getParent() == nullptr);
    REQUIRE(b.getParent() == nullptr);
    REQUIRE(c.getParent() == nullptr);
    REQUIRE(d.getParent() == nullptr);
    REQUIRE(e.getParent() == nullptr);
}


TEST_CASE("Widget - protection", "[unit][gui]") {
    FakeWidget p, f;
    Widget& w = f;
    AbstractWidget& a = f;
    p.addChild(w);
    w.setSize({10, 20});
    REQUIRE(a.getParent() == &p);
    auto size = a.getSize();
    REQUIRE_THAT(size.width, Catch::Matchers::WithinAbs(10.0, 1e-9));
    REQUIRE_THAT(size.height, Catch::Matchers::WithinAbs(20.0, 1e-9));
}
