// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "tools.hpp"

#include <vector>

class UpdateHandler;

/**
 * @brief Base class for widget tree nodes
 * 
 * Constant interface for widgets.
 * Provides read access to widget tree structure and widget sizes.
 */
class AbstractWidget {
public:
    AbstractWidget* getParent() {
        return parent;
    }

    UpdateHandler* getUpdateHandler() {
        return updateHandler;
    }

    Size<float> getSize() {
        return size;
    }

    virtual ~AbstractWidget() {}

    virtual std::vector<AbstractWidget*> getAllChildren() {
        return {};
    }

private:
    AbstractWidget* parent = nullptr;
    UpdateHandler* updateHandler = nullptr;
    Size<float> size;

    void setParent(AbstractWidget* widget) {
        parent = widget;
    }

    void setUpdateHandler(UpdateHandler* handler) {
        updateHandler = handler;
        for (auto& child : getAllChildren()) {
            child->setUpdateHandler(handler);
        }
    }

    void setSize(Size<float> value) {
        size = value;
    }

    template <typename T> friend class ParentWidget;
    friend class RootWidget;
    friend class Widget;
};


/**
 * @brief Base class for widgets that can have children
 * 
 * This class provides interface to add and remove children
 * and ensures that these children will have proper references to parents
 * and the update handler.
 */
template <typename T>
class ParentWidget: public AbstractWidget {
protected:
    void attach(T& widget) {
        widget.setParent(this);
        widget.setUpdateHandler(getUpdateHandler());
    }
    
    void detach(T& widget) {
        widget.setParent(nullptr);
        widget.setUpdateHandler(nullptr);
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


class UpdateHandler {};


/**
 * @brief Base class for the ultimate widget tree root
 * 
 * Provides the update handler for all attached widgets.
 */
class RootWidget: public ParentWidget<Widget> {
protected:
    void setUpdateHandler(UpdateHandler* handler) {
        AbstractWidget::setUpdateHandler(handler);
    }
};


/**
 * @brief Shows image
 * 
 * Basic element for all widget borders and background.
 */
class ImageWidget: public Widget {};
