/*
    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/**
 * \file nanogui/ColorComboBox.h
 *
 * \brief Simple combo box widget based on a popup button.
 */

#pragma once

#include <nanogui/popupbutton.h>
#include <nanogui/vscrollpanel.h>

NAMESPACE_BEGIN(nanogui)

/**
 * \class ColorComboBox ColorComboBox.h nanogui/ColorComboBox.h
 *
 * \brief Simple combo box widget based on a popup button.
 */
class NANOGUI_EXPORT ColorComboBox : public PopupButton {
public:
    /// Create an empty combo box
    ColorComboBox(Widget *parent);

    /// Create a new combo box with the given items
    ColorComboBox(Widget *parent, const std::vector<long> &items);

    /// The callback to execute for this ColorComboBox.
    std::function<void(int)> callback() const { return mCallback; }

    /// Sets the callback to execute for this ColorComboBox.
    void setCallback(const std::function<void(int)> &callback) { mCallback = callback; }

    /// The current index this ColorComboBox has selected.
    int selectedIndex() const { return mSelectedIndex; }

    /// Sets the current index this ColorComboBox has selected.
    void setSelectedIndex(int idx);

    /// Sets the items for this ColorComboBox.
    void setItems(const std::vector<long> &items);

    /// The items associated with this ColorComboBox.
    const std::vector<long> &items() const { return mItems; }

    /// Handles mouse scrolling events for this ColorComboBox.
    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel) override;

protected:
    /// The items associated with this ColorComboBox.
    std::vector<long> mItems;

    /// The callback for this ColorComboBox.
    std::function<void(int)> mCallback;

    /// The current index this ColorComboBox has selected.
    int mSelectedIndex;

    VScrollPanel *mPopupScroll;
    Widget *mButtonWrapper;
};

NAMESPACE_END(nanogui)
