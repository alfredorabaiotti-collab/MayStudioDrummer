#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

namespace may
{
namespace Colours
{
    const juce::Colour gold      { 0xffd8a94e };
    const juce::Colour goldDark  { 0xff8a6526 };
    const juce::Colour cream     { 0xfff1e6cf };
    const juce::Colour textDim   { 0xffb9ad97 };
    const juce::Colour panel     { 0xd2181512 };
    const juce::Colour panelLine { 0x33ffffff };
    const juce::Colour red       { 0xffa8322a };
    const juce::Colour lcd       { 0xff0f0e0c };
}

inline juce::Font uiFont (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}

class MayLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MayLookAndFeel()
    {
        setColour (juce::ComboBox::backgroundColourId, Colours::lcd);
        setColour (juce::ComboBox::textColourId, Colours::cream);
        setColour (juce::ComboBox::outlineColourId, Colours::panelLine);
        setColour (juce::ComboBox::arrowColourId, Colours::gold);
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1b1814));
        setColour (juce::PopupMenu::textColourId, Colours::cream);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours::goldDark);
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        setColour (juce::Label::textColourId, Colours::cream);
        setColour (juce::Slider::textBoxTextColourId, Colours::cream);
        setColour (juce::Slider::textBoxBackgroundColourId, Colours::lcd);
        setColour (juce::Slider::textBoxOutlineColourId, Colours::panelLine);
        setColour (juce::Slider::trackColourId, Colours::gold);
        setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a2622));
        setColour (juce::Slider::thumbColourId, Colours::cream);
        setColour (juce::TextButton::buttonColourId, juce::Colour (0xff23201c));
        setColour (juce::TextButton::buttonOnColourId, Colours::goldDark);
        setColour (juce::TextButton::textColourOffId, Colours::cream);
        setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        setColour (juce::ToggleButton::textColourId, Colours::cream);
        setColour (juce::ToggleButton::tickColourId, Colours::gold);
        setColour (juce::AlertWindow::backgroundColourId, juce::Colour (0xff1b1814));
        setColour (juce::AlertWindow::textColourId, Colours::cream);
        setColour (juce::AlertWindow::outlineColourId, Colours::gold);
        setColour (juce::TextEditor::backgroundColourId, Colours::lcd);
        setColour (juce::TextEditor::textColourId, Colours::cream);
        setColour (juce::TextEditor::outlineColourId, Colours::panelLine);
        setColour (juce::TextEditor::focusedOutlineColourId, Colours::gold);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return uiFont (14.0f, true); }
    juce::Font getPopupMenuFont() override { return uiFont (15.0f); }
    juce::Font getTextButtonFont (juce::TextButton&, int h) override { return uiFont (juce::jmin (14.0f, h * 0.5f), true); }
    juce::Font getLabelFont (juce::Label& l) override { return l.getFont(); }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override
    {
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
        const float size = juce::jmin (b.getWidth(), b.getHeight());
        b = b.withSizeKeepingCentre (size, size);
        const auto c = b.getCentre();
        const float r = size * 0.5f;

        // Arco valore
        juce::Path track;
        track.addCentredArc (c.x, c.y, r - 1.5f, r - 1.5f, 0.0f, a0, a1, true);
        g.setColour (juce::Colour (0x55000000));
        g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        juce::Path val;
        const float ang = a0 + pos * (a1 - a0);
        val.addCentredArc (c.x, c.y, r - 1.5f, r - 1.5f, 0.0f, a0, ang, true);
        g.setColour (Colours::gold);
        g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Anello bronzo
        auto ring = b.reduced (5.0f);
        juce::ColourGradient rg (juce::Colour (0xfff3d18a), ring.getX(), ring.getY(), juce::Colour (0xff6b4a1c), ring.getRight(), ring.getBottom(), false);
        rg.addColour (0.5, juce::Colour (0xffb98a3e));
        g.setGradientFill (rg);
        g.fillEllipse (ring);

        // Corpo scuro
        auto body = ring.reduced (3.5f);
        juce::ColourGradient bg (juce::Colour (0xff4a4640), body.getCentreX(), body.getY(), juce::Colour (0xff0d0c0b), body.getCentreX(), body.getBottom(), false);
        g.setGradientFill (bg);
        g.fillEllipse (body);
        g.setColour (juce::Colour (0x30ffffff));
        g.drawEllipse (body.reduced (1.0f), 1.0f);

        // Indice
        const float pr = body.getWidth() * 0.5f;
        juce::Line<float> ln (c.getPointOnCircumference (pr * 0.25f, ang), c.getPointOnCircumference (pr * 0.88f, ang));
        g.setColour (juce::Colours::white);
        g.drawLine (ln, 2.6f);
    }

    void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
        juce::ColourGradient gr (juce::Colour (0xff25221e), 0, 0, juce::Colour (0xff0e0d0b), 0, (float) h, false);
        g.setGradientFill (gr);
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (box.hasKeyboardFocus (false) ? Colours::gold : Colours::panelLine);
        g.drawRoundedRectangle (r, 5.0f, 1.0f);
        juce::Path arrow;
        const float ax = (float) w - 16.0f, ay = (float) h * 0.5f;
        arrow.addTriangle (ax - 5, ay - 3, ax + 5, ay - 3, ax, ay + 4);
        g.setColour (Colours::gold);
        g.fillPath (arrow);
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (8, 1, box.getWidth() - 28, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        const bool on = b.getToggleState();
        juce::ColourGradient gr (on ? juce::Colour (0xffe6bb63) : juce::Colour (0xff302c27), 0, r.getY(),
                                 on ? juce::Colour (0xff8a6526) : juce::Colour (0xff141210), 0, r.getBottom(), false);
        g.setGradientFill (gr);
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (over ? Colours::gold : Colours::panelLine);
        g.drawRoundedRectangle (r, 5.0f, down ? 2.0f : 1.0f);
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool) override
    {
        g.setFont (getTextButtonFont (b, b.getHeight()));
        g.setColour (b.getToggleState() ? juce::Colour (0xff1a1206) : Colours::cream);
        g.drawFittedText (b.getButtonText(), b.getLocalBounds().reduced (4, 2), juce::Justification::centred, 1);
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool) override
    {
        auto r = b.getLocalBounds().toFloat();
        auto led = r.removeFromLeft (r.getHeight()).reduced (r.getHeight() * 0.28f);
        const bool on = b.getToggleState();
        if (on)
        {
            g.setColour (Colours::gold.withAlpha (0.35f));
            g.fillEllipse (led.expanded (3.0f));
        }
        g.setColour (on ? Colours::gold : juce::Colour (0xff3a352f));
        g.fillEllipse (led);
        g.setColour (juce::Colour (0x66000000));
        g.drawEllipse (led, 1.0f);
        g.setColour (over ? juce::Colours::white : Colours::cream);
        g.setFont (uiFont (13.0f, true));
        g.drawFittedText (b.getButtonText(), r.toNearestInt().withTrimmedLeft (2), juce::Justification::centredLeft, 1);
    }

    void drawPopupMenuBackground (juce::Graphics& g, int w, int h) override
    {
        g.fillAll (findColour (juce::PopupMenu::backgroundColourId));
        g.setColour (Colours::goldDark);
        g.drawRect (0, 0, w, h);
    }
};
} // namespace may
