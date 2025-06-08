// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\widget.hpp>

#include <format>


TEST_CASE("ImageWidget", "[unit][gui]") {
    ImageWidget img("button");
    REQUIRE(img.getKey() == "button");
}


TEST_CASE("TextWidget", "[unit][gui]") {
    TextWidget text("caption");
    REQUIRE(text.getText() == "caption");
}


namespace {

    class FakeComposite : public CompositeWidget {
    public:
        FakeComposite(
            const std::string& name,
            uptrvector<Widget>&& children)
            : name(name), children(move(children)) {}

        void accept(WidgetVisitor& visitor) override {
            refvector<Widget> rv;
            for (auto& c : children) {
                rv.push_back(std::ref(*c));
            }
            visitor.visit(*this, rv);
        }

        const std::string& getName() const {
            return name;
        }
    
    private:
        std::string name;
        uptrvector<Widget> children;
    };

    class MockVisitor : public WidgetVisitor {
    public:
        std::vector<std::string> log;

        void visit(ImageWidget& widget) override {
            log.push_back(std::format("Image {}", widget.getKey()));
        }
        
        void visit(TextWidget& widget) override {
            log.push_back(std::format("Text {}", widget.getText()));
        }
        
        void visit(CompositeWidget& widget, refvector<Widget> children) override {
            const std::string& name = (dynamic_cast<FakeComposite*>(&widget))->getName();
            log.push_back(std::format("Composite {} start", name));
            for (auto& w: children) {
                w.get().accept(*this);
            }
            log.push_back(std::format("Composite {} finish", name));
        }
        
        void visit(WindowWidget& widget, refvector<Widget> children) override {}
    };

}


TEST_CASE("WidgetVisitor elementary", "[unit][gui]") {
    MockVisitor visitor;
    ImageWidget img("button");
    TextWidget text("caption");
    img.accept(visitor);
    text.accept(visitor);
    REQUIRE(visitor.log == std::vector<std::string>{
        "Image button",
        "Text caption",
    });
}


TEST_CASE("WidgetVisitor empty composite", "[unit][gui]") {
    MockVisitor visitor;
    FakeComposite composite("outer", uptrvector<Widget>{});
    composite.accept(visitor);
    REQUIRE(visitor.log == std::vector<std::string>{
        "Composite outer start",
        "Composite outer finish"});
}


TEST_CASE("WidgetVisitor simple composite", "[unit][gui]") {
    MockVisitor visitor;
    uptrvector<Widget> children;
    children.push_back(move(std::make_unique<ImageWidget>("button")));
    children.push_back(move(std::make_unique<TextWidget>("caption")));
    FakeComposite composite("outer", move(children));
    composite.accept(visitor);
    REQUIRE(visitor.log == std::vector<std::string>{
        "Composite outer start",
        "Image button",
        "Text caption",
        "Composite outer finish",
    });
}
