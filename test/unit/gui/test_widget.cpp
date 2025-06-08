// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\widget.hpp>

#include <format>


TEST_CASE("ImageWidget", "[unit][gui]") {
    ImageWidget img("button");
    REQUIRE(img.getId() == "button");
}


namespace {
    
    class MockVisitor : public WidgetVisitor {
    public:
        std::vector<std::string> log;

        void visit(ImageWidget& widget) override {
            log.push_back(std::format("Image {}", widget.getId()));
        }
        
        void visit(TextWidget& widget) override {}
        
        void visit(CompositeWidget& widget, refvector<Widget> children) override {}
        
        void visit(WindowWidget& widget, refvector<Widget> children) override {}
    };

}


TEST_CASE("WidgetVisitor", "[unit][gui]") {
    MockVisitor visitor;
    ImageWidget img("button");
    img.accept(visitor);
    REQUIRE(visitor.log == std::vector<std::string>{"Image button"});
}
