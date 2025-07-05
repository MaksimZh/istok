// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "tools.hpp"
#include "tree.hpp"


class UpdateHandler;

/**
 * @brief Base class for widget tree nodes
 * 
 * Constant interface for widgets.
 * Provides read access to widget tree structure and widget sizes.
 */
class AbstractWidget: private Node<AbstractWidget> {
public:
    UpdateHandler* getUpdateHandler() {
        return updateHandler;
    }

    Size<float> getSize() {
        return size;
    }

    virtual ~AbstractWidget() {}

    NodeContainer<AbstractWidget>::Range getChildren() {
        return Node::getChildren();
    }

    NodeContainer<AbstractWidget>::Range getVisibleChildren() {
        return Node::getVisibleChildren();
    }

private:
    UpdateHandler* updateHandler = nullptr;
    Size<float> size;

    void setUpdateHandler(UpdateHandler* handler) {
        updateHandler = handler;
        for (auto& child : getChildren()) {
            child.setUpdateHandler(handler);
        }
    }

    void setSize(Size<float> value) {
        size = value;
    }

    template <typename T> friend class ParentWidget;
    template <typename T> friend class RootWidget;
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
class ParentWidget: public virtual AbstractWidget {
protected:
    void addChild(T& widget) {
        Node::addChild(widget);
    }
};


/**
 * @brief Base class for most widgets
 * 
 * Can become any part of widget tree.
 * Can be resized.
 * All widgets (except Screen) should be derived from this class.
 */
class Widget: public virtual ParentWidget<Widget> {
public:
    using AbstractWidget::setSize;
};


class UpdateHandler {};


/**
 * @brief Base class for the ultimate widget tree root
 * 
 * Provides the update handler for all attached widgets.
 */
template <typename T>
class RootWidget: public virtual ParentWidget<T> {
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
