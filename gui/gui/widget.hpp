#pragma once

class Widget {
public:
    Widget* getBase() {
        return nullptr;
    }
    
    void setBase(Widget* widget) {
        base = widget;
    }

private:
    Widget* base = nullptr;
};