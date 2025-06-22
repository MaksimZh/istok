// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "tools.hpp"

/**
 * @brief Parent class for all widgets
 * 
 * Immutable interface for widgets.
 * Provides read access to widget tree structure and widget sizes.
 * Derived classes have write access to the size.
 */
class Widget {
public:
    Widget* getBase() {
        return base;
    }

    Size<float> getSize() {
        return size;
    }

protected:
    void setSize(Size<float> value) {
        size = value;
    }

private:
    Widget* base = nullptr;
    Size<float> size;

    template <typename T> friend class BaseWidget;
    friend class MutableWidget;

    void setBase(Widget* widget) {
        base = widget;
    }
};


/**
 * @brief Parent class for widgets that can have parts
 * 
 * This class provides interface to add and remove parts and ensures
 * that these parts will have proper references to base.
 * 
 * Note that it is possible to create BaseWidget
 * that cannot be attached to any widget.
 * This template is designed specially for Screen widget
 * that is always a root of widget tree and can control its resizing.
 */
template <typename T>
class BaseWidget: public Widget {
public:
    void attach(T& part) {
        part.setBase(this);
    }
    
    void detach(T& part) {
        part.setBase(nullptr);
    }
};


/**
 * @brief Parent class for all widgets
 * 
 * Can become any part of widget tree.
 * Can be resized.
 * All widgets (except Screen) should be derived from this class.
 */
class MutableWidget: public BaseWidget<MutableWidget> {
public:
    using Widget::setSize;
};


/**
 * @brief Shows image
 * 
 * Basic element for all widget borders and background.
 */
class ImageWidget: public MutableWidget {};
