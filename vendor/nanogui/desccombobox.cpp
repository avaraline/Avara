/*
    src/DescComboBox.cpp -- simple combo box widget based on a popup button

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/desccombobox.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <cassert>

NAMESPACE_BEGIN(nanogui)

DescComboBox::DescComboBox(Widget *parent) : PopupButton(parent), mSelectedIndex(0) {
    mPopupScroll = new VScrollPanel(mPopup);
    mButtonWrapper = new Widget(mPopupScroll);
    mButtonWrapper->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 10, 10));
}

DescComboBox::DescComboBox(Widget *parent, const std::vector<std::string> &items)
    : DescComboBox(parent) {
    setItems(items);
}

DescComboBox::DescComboBox(Widget *parent, const std::vector<std::string> &items, const std::vector<std::string> &itemsShort)
    : DescComboBox(parent) {
    setItems(items, itemsShort);
}

void DescComboBox::setSelectedIndex(int idx) {
    if (mItemsShort.empty())
        return;
    const std::vector<Widget *> &children = mButtonWrapper->children();
    ((Button *) children[mSelectedIndex])->setPushed(false);
    ((Button *) children[idx])->setPushed(true);
    mSelectedIndex = idx;
    //setCaption(mItemsShort[idx]);
    setCaption(mItems[idx]);
}

void DescComboBox::setItems(const std::vector<std::string> &items, const std::vector<std::string> &itemsShort) {
    assert(items.size() == itemsShort.size());
    mItems = items;
    mItemsShort = itemsShort;
    if (mSelectedIndex < 0 || mSelectedIndex >= (int) items.size())
        mSelectedIndex = 0;
    while (mButtonWrapper->childCount() != 0)
        mButtonWrapper->removeChild(mButtonWrapper->childCount()-1);
    int index = 0;
    for (const auto &str: items) {
        Button *button = new Button(mButtonWrapper, str);
        button->setFlags(Button::RadioButton);
        button->setCallback([&, index] {
            mSelectedIndex = index;
            setCaption(mItems[index]);
            setPushed(false);
            popup()->setVisible(false);
            if (mCallback)
                mCallback(index);
        });
        Label *label = new Label(mButtonWrapper, mItemsShort[index]);
        label->setFixedWidth(450);
        index++;
    }
    setSelectedIndex(mSelectedIndex);
    setNeedsLayout();
}

bool DescComboBox::scrollEvent(const Vector2i &p, const Vector2f &rel) {
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
