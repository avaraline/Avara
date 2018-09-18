/*
    src/ColorComboBox.cpp -- simple combo box widget based on a popup button

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/colorcombobox.h>
#include <nanogui/layout.h>
#include <cassert>

NAMESPACE_BEGIN(nanogui)

ColorComboBox::ColorComboBox(Widget *parent) : PopupButton(parent, " "), mSelectedIndex(0) {
    mPopup->setFixedWidth(45);
    mPopupScroll = new VScrollPanel(mPopup);
    mButtonWrapper = new Widget(mPopupScroll);
    mButtonWrapper->setLayout(new GroupLayout(10));
}

ColorComboBox::ColorComboBox(Widget *parent, const std::vector<long> &items)
    : ColorComboBox(parent) {
    setItems(items);
    mSelectedIndex = 0;
}

void ColorComboBox::setSelectedIndex(int idx) {
    const std::vector<Widget *> &children = mButtonWrapper->children();
    ((Button *) children[mSelectedIndex])->setPushed(false);
    ((Button *) children[idx])->setPushed(true);
    long color_long = mItems[idx];
    setBackgroundColor(Color(
            (int)(color_long >> 16) & 0xff,
            (int)(color_long >> 8) & 0xff,
            (int)color_long & 0xff, 
            255));
    mSelectedIndex = idx;
}

void ColorComboBox::setItems(const std::vector<long> &items) {
    mItems = items;
    if (mSelectedIndex < 0 || mSelectedIndex >= (int) items.size())
        mSelectedIndex = 0;
    while (mButtonWrapper->childCount() != 0)
        mButtonWrapper->removeChild(mButtonWrapper->childCount()-1);
    int index = 0;
    for (const auto &color_long: items) {
        Button *button = new Button(mButtonWrapper, " ");
        button->setFlags(Button::RadioButton);
        button->setCallback([&, index] {
            //mSelectedIndex = index;
            setSelectedIndex(index);
            setPushed(false);
            popup()->setVisible(false);
            if (mCallback)
                mCallback(index);
        });
        button->setBackgroundColor(Color(
            (int)(color_long >> 16) & 0xff,
            (int)(color_long >> 8) & 0xff,
            (int)color_long & 0xff, 
            255));
        index++;
    }
    setSelectedIndex(mSelectedIndex);
    setNeedsLayout();
}

bool ColorComboBox::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    if (rel.y < 0) {
        setSelectedIndex(std::min(mSelectedIndex+1, (int)(items().size()-1)));
        if (mCallback)
            mCallback(mSelectedIndex);
        return true;
    } else if (rel.y > 0) {
        setSelectedIndex(std::max(mSelectedIndex-1, 0));
        if (mCallback)
            mCallback(mSelectedIndex);
        return true;
    }
    return Widget::scrollEvent(p, rel);
}

NAMESPACE_END(nanogui)
