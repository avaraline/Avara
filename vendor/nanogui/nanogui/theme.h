/*
    The text box widget was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/**
 * \file nanogui/theme.h
 *
 * \brief Storage class for basic theme-related properties.
 */

#pragma once

#include <nanogui/common.h>
#include <nanogui/object.h>
#include <stdexcept>

NAMESPACE_BEGIN(nanogui)

/**
 * \class Theme theme.h nanogui/theme.h
 *
 * \brief Storage class for basic theme-related properties.
 */
class NANOGUI_EXPORT Theme : public Object {
public:
    Theme(NVGcontext *ctx);

    /* Fonts */
    /// The standard font face (default: ``"sans"`` from ``resources/roboto_regular.ttf``).
    int mFontNormal;
    /// The bold font face (default: ``"sans-bold"`` from ``resources/roboto_regular.ttf``).
    int mFontBold;
    /// The icon font face (default: ``"icons"`` from ``resources/entypo.ttf``).
    int mFontIcons;
    /// The monospace font face (default: ``"mono"`` from ``profont.ttf``).
    int mFontMono;
    /**
     * The amount of scaling that is applied to each icon to fit the size of
     * NanoGUI widgets.  The default value is ``0.77f``, setting to e.g. higher
     * than ``1.0f`` is generally discouraged.
     */
    float mIconScale;

    /* Spacing-related parameters */
    /// The font size for all widgets other than buttons and textboxes (default: `` 16``).
    int mStandardFontSize;
    /// The font size for buttons (default: ``20``).
    int mButtonFontSize;
    /// The font size for text boxes (default: ``20``).
    int mTextBoxFontSize;
    /// Rounding radius for Window widget corners (default: ``2``).
    int mWindowCornerRadius;
    /// Default size of Window widget titles (default: ``30``).
    int mWindowHeaderHeight;
    /// Size of drop shadow rendered behind the Window widgets (default: ``10``).
    int mWindowDropShadowSize;
    /// Rounding radius for Button (and derived types) widgets (default: ``2``).
    int mButtonCornerRadius;
    /// The border width for TabHeader widgets (default: ``0.75f``).
    float mTabBorderWidth;
    /// The inner margin on a TabHeader widget (default: ``5``).
    int mTabInnerMargin;
    /// The minimum size for buttons on a TabHeader widget (default: ``20``).
    int mTabMinButtonWidth;
    /// The maximum size for buttons on a TabHeader widget (default: ``160``).
    int mTabMaxButtonWidth;
    /// Used to help specify what lies "in bound" for a TabHeader widget (default: ``20``).
    int mTabControlWidth;
    /// The amount of horizontal padding for a TabHeader widget (default: ``10``).
    int mTabButtonHorizontalPadding;
    /// The amount of vertical padding for a TabHeader widget (default: ``2``).
    int mTabButtonVerticalPadding;

    /* Generic colors */
    /**
     * The color of the drop shadow drawn behind widgets
     * (default: intensity=``0``, alpha=``128``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mDropShadow;
    /**
     * The transparency color
     * (default: intensity=``0``, alpha=``0``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mTransparent;
    /**
     * The dark border color
     * (default: intensity=``29``, alpha=``255``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mBorderDark;
    /**
     * The light border color
     * (default: intensity=``92``, alpha=``255``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mBorderLight;
    /**
     * The medium border color
     * (default: intensity=``35``, alpha=``255``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mBorderMedium;
    /**
     * The text color
     * (default: intensity=``255``, alpha=``160``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mTextColor;
    /**
     * The disable dtext color
     * (default: intensity=``255``, alpha=``80``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mDisabledTextColor;
    /**
     * The text shadow color
     * (default: intensity=``0``, alpha=``160``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mTextColorShadow;
    /// The icon color (default: \ref nanogui::Theme::mTextColor).
    Color mIconColor;

    /* Button colors */
    /**
     * The top gradient color for buttons in focus
     * (default: intensity=``64``, alpha=``255``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mButtonGradientTopFocused;
    /**
     * The bottom gradient color for buttons in focus
     * (default: intensity=``48``, alpha=``255``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mButtonGradientBotFocused;
    /**
     * The top gradient color for buttons not in focus
     * (default: intensity=``74``, alpha=``255``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mButtonGradientTopUnfocused;
    /**
     * The bottom gradient color for buttons not in focus
     * (default: intensity=``58``, alpha=``255``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mButtonGradientBotUnfocused;
    /**
     * The top gradient color for buttons currently pushed
     * (default: intensity=``41``, alpha=``255``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mButtonGradientTopPushed;
    /**
     * The bottom gradient color for buttons currently pushed
     * (default: intensity=``29``, alpha=``255``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mButtonGradientBotPushed;

    /* Window colors */
    /**
     * The fill color for a Window that is not in focus
     * (default: intensity=``43``, alpha=``230``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mWindowFillUnfocused;
    /**
     * The fill color for a Window that is in focus
     * (default: intensity=``45``, alpha=``230``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mWindowFillFocused;
    /**
     * The title color for a Window that is not in focus
     * (default: intensity=``220``, alpha=``160``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mWindowTitleUnfocused;
    /**
     * The title color for a Window that is in focus
     * (default: intensity=``255``, alpha=``190``; see \ref nanogui::Color::Color(int,int)).
     */
    Color mWindowTitleFocused;

    /**
     * The top gradient color for Window headings
     * (default: \ref nanogui::Theme::mButtonGradientTopUnfocused).
     */
    Color mWindowHeaderGradientTop;
    /**
     * The bottom gradient color for Window headings
     * (default: \ref nanogui::Theme::mButtonGradientBotUnfocused).
     */
    Color mWindowHeaderGradientBot;
    /// The Window header top separation color (default: \ref nanogui::Theme::mBorderLight).
    Color mWindowHeaderSepTop;
    /// The Window header bottom separation color (default: \ref nanogui::Theme::mBorderDark).
    Color mWindowHeaderSepBot;

    /**
     * The popup window color
     * (default: intensity=``50``, alpha=``255``; see \ref nanogui::Color::Color(int,int))).
     */
    Color mWindowPopup;
    /**
     * The transparent popup window color
     * (default: intensity=``50``, alpha=``0``; see \ref nanogui::Color::Color(int,int))).
     */
    Color mWindowPopupTransparent;

    /// Icon to use for CheckBox widgets (default: ``ENTYPO_ICON_CHECK``).
    int mCheckBoxIcon;
    /// Icon to use for informational MessageDialog widgets (default: ``ENTYPO_ICON_INFO_WITH_CIRCLE``).
    int mMessageInformationIcon;
    /// Icon to use for interrogative MessageDialog widgets (default: ``ENTYPO_ICON_HELP_WITH_CIRCLE``).
    int mMessageQuestionIcon;
    /// Icon to use for warning MessageDialog widgets (default: ``ENTYPO_ICON_WARNING``).
    int mMessageWarningIcon;
    /// Icon to use on MessageDialog alt button (default: ``ENTYPO_ICON_CIRCLE_WITH_CROSS``).
    int mMessageAltButtonIcon;
    /// Icon to use on MessageDialog primary button (default: ``ENTYPO_ICON_CHECK``).
    int mMessagePrimaryButtonIcon;
    /// Icon to use for PopupButton widgets opening to the right (default: ``ENTYPO_ICON_CHEVRON_RIGHT``).
    int mPopupChevronRightIcon;
    /// Icon to use for PopupButton widgets opening to the left (default: ``ENTYPO_ICON_CHEVRON_LEFT``).
    int mPopupChevronLeftIcon;
    /// Icon to indicate hidden tabs to the left on a TabHeader (default: ``ENTYPO_ICON_ARROW_BOLD_LEFT``).
    int mTabHeaderLeftIcon;
    /// Icon to indicate hidden tabs to the right on a TabHeader (default: ``ENTYPO_ICON_ARROW_BOLD_RIGHT``).
    int mTabHeaderRightIcon;
    /// Icon to use when a TextBox has an up toggle (e.g. IntBox) (default: ``ENTYPO_ICON_CHEVRON_UP``).
    int mTextBoxUpIcon;
    /// Icon to use when a TextBox has a down toggle (e.g. IntBox) (default: ``ENTYPO_ICON_CHEVRON_DOWN``).
    int mTextBoxDownIcon;

protected:
    /// Default destructor does nothing; allows for inheritance.
    virtual ~Theme() { };

};

NAMESPACE_END(nanogui)
