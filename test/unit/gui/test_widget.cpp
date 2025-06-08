// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\widget.hpp>

#include <format>


TEST_CASE("ImageWidget", "[unit][gui]") {
    ImageWidget img("button");
    REQUIRE(img.getId() == "button");
}


TEST_CASE("TextWidget", "[unit][gui]") {
    TextWidget text("caption");
    REQUIRE(text.getText() == "caption");
}


namespace {
    
    class MockVisitor : public WidgetVisitor {
    public:
        std::vector<std::string> log;

        void visit(ImageWidget& widget) override {
            log.push_back(std::format("Image {}", widget.getId()));
        }
        
        void visit(TextWidget& widget) override {
            log.push_back(std::format("Text {}", widget.getText()));
        }
        
        void visit(CompositeWidget& widget, refvector<Widget> children) override {}
        
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
        "Text caption"});
}
