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
            std::vector<IdWidget> children)
            : name(name), children(children) {}

        void accept(WidgetVisitor& visitor) override {
            visitor.visit(*this, children);
        }

        const std::string& getName() const {
            return name;
        }
    
    private:
        std::string name;
        std::vector<IdWidget> children;
    };


    class FakeWindow : public WindowWidget {
    public:
        FakeWindow(const std::string& title, Widget& child)
            : titleBar(title), child(child) {}
        
        void accept(WidgetVisitor& visitor) override {
            visitor.visit(*this, {{"title", titleBar}, {"child", child}});
        }

        const std::string& getTitle() const {
            return titleBar.getText();
        }

    private:
        TextWidget titleBar;
        Widget& child;
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
        
        void visit(
                CompositeWidget& widget,
                std::vector<IdWidget> children) override {
            const std::string& name = (dynamic_cast<FakeComposite*>(&widget))->getName();
            log.push_back(std::format("Composite {} start", name));
            for (auto& w: children) {
                w.second.accept(*this);
            }
            log.push_back(std::format("Composite {} finish", name));
        }
        
        void visit(
                WindowWidget& widget,
                std::vector<IdWidget> children) override {
            const std::string& title = (dynamic_cast<FakeWindow*>(&widget))->getTitle();
            log.push_back(std::format("Window {} start", title));
            for (auto& w: children) {
                w.second.accept(*this);
            }
            log.push_back(std::format("Window {} finish", title));
        }
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
    FakeComposite composite("outer", {});
    composite.accept(visitor);
    REQUIRE(visitor.log == std::vector<std::string>{
        "Composite outer start",
        "Composite outer finish"});
}


TEST_CASE("WidgetVisitor simple composite", "[unit][gui]") {
    MockVisitor visitor;
    ImageWidget img("button");
    TextWidget text("caption");
    FakeComposite composite("outer", {{"a", img}, {"b", text}});
    composite.accept(visitor);
    REQUIRE(visitor.log == std::vector<std::string>{
        "Composite outer start",
        "Image button",
        "Text caption",
        "Composite outer finish",
    });
}


TEST_CASE("WidgetVisitor nested composite", "[unit][gui]") {
    MockVisitor visitor;
    ImageWidget img("button");
    TextWidget text("caption");
    FakeComposite inner("inner", {{"a", img}, {"b", text}});
    ImageWidget img1("field");
    TextWidget text1("label");
    FakeComposite composite("outer", {{"a", img1}, {"b", inner}, {"c", text1}});
    composite.accept(visitor);
    REQUIRE(visitor.log == std::vector<std::string>{
        "Composite outer start",
        "Image field",
        "Composite inner start",
        "Image button",
        "Text caption",
        "Composite inner finish",
        "Text label",
        "Composite outer finish",
    });
}


TEST_CASE("WidgetVisitor window", "[unit][gui]") {
    MockVisitor visitor;
    ImageWidget img("button");
    TextWidget text("caption");
    FakeComposite composite("panel", {{"a", img}, {"b", text}});
    FakeWindow window("main", composite);
    window.accept(visitor);
    REQUIRE(visitor.log == std::vector<std::string>{
        "Window main start",
        "Text main",
        "Composite panel start",
        "Image button",
        "Text caption",
        "Composite panel finish",
        "Window main finish",
    });
}
