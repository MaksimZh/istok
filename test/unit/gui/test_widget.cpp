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
            : name(name), children(std::move(children)) {}

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


    class FakeWindow : public WindowWidget {
    public:
        FakeWindow(const std::string& title, std::unique_ptr<Widget>&& child)
            : titleBar(title), child(std::move(child)) {}
        
        void accept(WidgetVisitor& visitor) override {
            visitor.visit(*this, {titleBar, std::ref(*child)});
        }

        const std::string& getTitle() const {
            return titleBar.getText();
        }

    private:
        TextWidget titleBar;
        std::unique_ptr<Widget> child;
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
        
        void visit(WindowWidget& widget, refvector<Widget> children) override {
            const std::string& title = (dynamic_cast<FakeWindow*>(&widget))->getTitle();
            log.push_back(std::format("Window {} start", title));
            for (auto& w: children) {
                w.get().accept(*this);
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
    FakeComposite composite("outer", uptrvector<Widget>{});
    composite.accept(visitor);
    REQUIRE(visitor.log == std::vector<std::string>{
        "Composite outer start",
        "Composite outer finish"});
}


TEST_CASE("WidgetVisitor simple composite", "[unit][gui]") {
    MockVisitor visitor;
    uptrvector<Widget> children;
    children.push_back(std::move(std::make_unique<ImageWidget>("button")));
    children.push_back(std::move(std::make_unique<TextWidget>("caption")));
    FakeComposite composite("outer", std::move(children));
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
    uptrvector<Widget> children;
    children.push_back(std::move(std::make_unique<ImageWidget>("button")));
    children.push_back(std::move(std::make_unique<TextWidget>("caption")));
    std::unique_ptr<FakeComposite> inner =
        std::make_unique<FakeComposite>("inner", std::move(children));
    children.push_back(std::move(std::make_unique<ImageWidget>("field")));
    children.push_back(std::move(inner));
    children.push_back(std::move(std::make_unique<TextWidget>("label")));
    FakeComposite composite("outer", std::move(children));
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
    uptrvector<Widget> children;
    children.push_back(std::move(std::make_unique<ImageWidget>("button")));
    children.push_back(std::move(std::make_unique<TextWidget>("caption")));
    std::unique_ptr<FakeComposite> composite =
        std::make_unique<FakeComposite>("panel", std::move(children));
    FakeWindow window("main", std::move(composite));
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
