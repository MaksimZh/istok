// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "tools.hpp"

/**
 * @brief Base class for widget tree nodes
 * 
 * Constant interface for widgets.
 * Provides read access to widget tree structure and widget sizes.
 */
class AbstractWidget {
public:
    AbstractWidget* getBase() {
        return base;
    }

    Size<float> getSize() {
        return size;
    }

private:
    AbstractWidget* base = nullptr;
    Size<float> size;

    void setBase(AbstractWidget* widget) {
        base = widget;
    }

    void setSize(Size<float> value) {
        size = value;
    }

    template <typename T> friend class ParentWidget;
    friend class Widget;
};


/**
 * @brief Base class for widgets that can have children
 * 
 * This class provides interface to add and remove children and ensures
 * that these children will have proper references to parents.
 * 
 * Note that it is possible to create ParentWidget
 * that cannot be attached to any widget.
 * This template is designed specially for Screen widget
 * that is always a root of widget tree and can control its resizing.
 */
template <typename T>
class ParentWidget: public AbstractWidget {
protected:
    void attach(T& part) {
        part.setBase(this);
    }
    
    void detach(T& part) {
        part.setBase(nullptr);
    }
};


/**
 * @brief Base class for most widgets
 * 
 * Can become any part of widget tree.
 * Can be resized.
 * All widgets (except Screen) should be derived from this class.
 */
class Widget: public ParentWidget<Widget> {
public:
    using AbstractWidget::setSize;
};


/**
 * @brief Shows image
 * 
 * Basic element for all widget borders and background.
 */
class ImageWidget: public Widget {};
