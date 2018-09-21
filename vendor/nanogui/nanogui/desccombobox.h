/*
    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/**
 * \file nanogui/DescComboBox.h
 *
 * \brief Simple combo box widget based on a popup button.
 */

#pragma once

#include <nanogui/popupbutton.h>
#include <nanogui/vscrollpanel.h>

NAMESPACE_BEGIN(nanogui)

/**
 * \class DescComboBox DescComboBox.h nanogui/DescComboBox.h
 *
 * \brief Simple combo box widget based on a popup button.
 */
class NANOGUI_EXPORT DescComboBox : public PopupButton {
public:
    /// Create an empty combo box
    DescComboBox(Widget *parent);

    /// Create a new combo box with the given items
    DescComboBox(Widget *parent, const std::vector<std::string> &items);

    /**
     * \brief Create a new combo box with the given items, providing both short and
     * long descriptive labels for each item
     */
    DescComboBox(Widget *parent, const std::vector<std::string> &items,
             const std::vector<std::string> &itemsShort);

    /// The callback to execute for this DescComboBox.
    std::function<void(int)> callback() const { return mCallback; }

    /// Sets the callback to execute for this DescComboBox.
    void setCallback(const std::function<void(int)> &callback) { mCallback = callback; }

    /// The current index this DescComboBox has selected.
    int selectedIndex() const { return mSelectedIndex; }

    /// Sets the current index this DescComboBox has selected.
    void setSelectedIndex(int idx);

    /// Sets the items for this DescComboBox, providing both short and long descriptive lables for each item.
    void setItems(const std::vector<std::string> &items, const std::vector<std::string> &itemsShort);

    /// Sets the items for this DescComboBox.
    void setItems(const std::vector<std::string> &items) { setItems(items, items); }

    /// The items associated with this DescComboBox.
    const std::vector<std::string> &items() const { return mItems; }

    /// The short descriptions associated with this DescComboBox.
    const std::vector<std::string> &itemsShort() const { return mItemsShort; }

    /// Handles mouse scrolling events for this DescComboBox.
    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel) override;

protected:
    /// The items associated with this DescComboBox.
    std::vector<std::string> mItems;

    /// The short descriptions of items associated with this DescComboBox.
    std::vector<std::string> mItemsShort;

    /// The callback for this DescComboBox.
    std::function<void(int)> mCallback;

    /// The current index this DescComboBox has selected.
    int mSelectedIndex;

    VScrollPanel *mPopupScroll;
    Widget *mButtonWrapper;
};

NAMESPACE_END(nanogui)
