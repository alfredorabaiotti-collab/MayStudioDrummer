#include "PluginEditor.h"
#include "BinaryData.h"

using namespace may;

namespace
{
const int displayOrder[] = { Crash, China, Ride, RideBell, HHOpen, HHClosed, HHPedal, Tambourine, Shaker, Cowbell,
                             TomHi, TomMid, TomFloor, Clap, SideStick, Snare, Kick };

juce::Colour pieceColour (int p)
{
    switch (groupOf (p))
    {
        case GKick:    return juce::Colour (0xffe07a3a);
        case GSnare:   return juce::Colour (0xfff1e6cf);
        case GHats:    return juce::Colour (0xffd8a94e);
        case GToms:    return juce::Colour (0xffc9583e);
        case GCymbals: return juce::Colour (0xffe9d48a);
        default:       return juce::Colour (0xff9fc3a8);
    }
}

juce::Colour kindColour (int k)
{
    switch (k)
    {
        case 1:  return juce::Colour (0xffd8a94e);   // riff
        case 2:  return juce::Colour (0xffc9583e);   // fill
        case 3:  return juce::Colour (0xff8f8a80);   // finale
        case 4:  return juce::Colour (0xff6c8fa6);   // count-in
        default: return juce::Colour (0xff6d5236);   // main
    }
}

struct SectionDef { int page; juce::Rectangle<int> r; const char* num; const char* title; };
const SectionDef sections[] = {
    { 0, { 10, 8, 1136, 96 },    "", "" },
    { 0, { 10, 112, 1136, 84 },  "4", "PATTERN PRESET" },
    { 0, { 10, 204, 1136, 270 }, "5", "MAIN  /  RIFF  /  FINALE" },
    { 0, { 10, 482, 1136, 112 }, "",  "HUMANIZE E SUONO" },
    { 0, { 10, 602, 1136, 226 }, "6", "MAY DRUMMER" },
    { 1, { 10, 8, 1136, 140 },   "",  "STRUTTURA DELLA TRACCIA" },
    { 1, { 10, 156, 1136, 420 }, "",  "ANTEPRIMA SONG" },
    { 1, { 10, 584, 1136, 244 }, "",  "IMPORTA NELLA DAW" },
    { 0 + 2, { 10, 8, 1136, 330 },  "", "MAPPA NOTE MIDI" },
    { 2, { 10, 346, 1136, 482 }, "",  "KIT, CANALE E CONTROLLO" },
    { 3, { 10, 8, 1136, 820 }, "",  "BROWSER DEI PRESET" },
};
} // namespace

// ============================================================ DragZone
void DragZone::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    juce::ColourGradient gr (juce::Colour (hover ? 0xfff0c871 : 0xffdcae55), 0, r.getY(), juce::Colour (0xff7d5a20), 0, r.getBottom(), false);
    g.setGradientFill (gr);
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (juce::Colour (0x88000000));
    g.drawRoundedRectangle (r, 8.0f, 1.2f);

    auto inner = r.reduced (12.0f, 6.0f);
    const float sc = juce::jlimit (0.45f, 1.0f, r.getHeight() / 60.0f);
    auto icon = inner.removeFromLeft (juce::jmin (36.0f * sc, inner.getHeight()));
    juce::Path arrow;
    const auto c = icon.getCentre();
    arrow.addArrow ({ c.x, c.y - 12.0f * sc, c.x, c.y + 10.0f * sc }, 6.0f * sc, 16.0f * sc, 11.0f * sc);
    g.setColour (juce::Colour (0xff1a1206));
    g.fillPath (arrow);
    g.fillRect (c.x - 11.0f * sc, c.y + 13.0f * sc, 22.0f * sc, juce::jmax (1.5f, 3.0f * sc));

    const bool tall = r.getHeight() > 70.0f;
    if (r.getHeight() < 34.0f)
    {
        g.setFont (uiFont (12.0f, true));
        g.drawFittedText (title, inner.toNearestInt(), juce::Justification::centredLeft, 1);
        return;
    }
    g.setFont (uiFont (tall ? 17.0f : 14.0f, true));
    g.drawFittedText (title, inner.removeFromTop (inner.getHeight() * 0.55f).toNearestInt(), juce::Justification::bottomLeft, 1);
    g.setFont (uiFont (tall ? 13.0f : 11.5f));
    g.setColour (juce::Colour (0xff3a2a10));
    g.drawFittedText (subtitle, inner.toNearestInt(), juce::Justification::topLeft, 2);
}

// ============================================================ PatternView
void PatternView::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xe60b0a09));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (Colours::panelLine);
    g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);
    if (! source) return;
    const auto& seq = source();

    std::vector<int> rows;
    for (int p : displayOrder)
        for (auto& e : seq.events) if (e.piece == p) { rows.push_back (p); break; }
    if (rows.empty()) return;

    auto area = r.reduced (6.0f, 5.0f);
    const float labelW = compact ? 64.0f : 118.0f;
    auto labels = area.removeFromLeft (labelW);

    if (seq.barKinds.size() > 1 && ! compact)
    {
        auto strip = area.removeFromTop (8.0f);
        const float bw = strip.getWidth() / (float) seq.barKinds.size();
        for (size_t i = 0; i < seq.barKinds.size(); ++i)
        {
            g.setColour (kindColour (seq.barKinds[i]));
            g.fillRect (strip.getX() + bw * (float) i + 0.5f, strip.getY(), bw - 1.0f, strip.getHeight());
        }
        area.removeFromTop (3.0f);
        labels.removeFromTop (11.0f);
    }

    const float rowH = area.getHeight() / (float) rows.size();
    const double len = juce::jmax (0.01, seq.lengthPPQ);

    if (compact)
    {
        g.setColour (accent);
        g.setFont (uiFont (12.5f, true));
        g.drawFittedText (title, labels.toNearestInt().reduced (2, 0), juce::Justification::centredLeft, 2);
    }

    for (size_t i = 0; i < rows.size(); ++i)
    {
        const float y = area.getY() + rowH * (float) i;
        if (i % 2 == 0) { g.setColour (juce::Colour (0x0affffff)); g.fillRect (area.getX(), y, area.getWidth(), rowH); }
        if (! compact)
        {
            g.setColour (Colours::textDim);
            g.setFont (uiFont (juce::jlimit (9.0f, 12.5f, rowH * 0.8f)));
            juce::String txt (pieceName (rows[i]));
            if (noteOf) txt << "  " << noteOf (rows[i]);
            g.drawText (txt, juce::Rectangle<float> (labels.getX(), y, labelW - 6.0f, rowH), juce::Justification::centredRight);
        }
    }

    // Griglia: battute e pulsazioni
    const double beatPPQ = 4.0 / juce::jmax (1, seq.meterDen);
    for (double q = 0.0; q < len - 1e-6; q += beatPPQ)
    {
        const float x = area.getX() + (float) (q / len) * area.getWidth();
        const bool isBar = std::fmod (q + 1e-6, seq.barPPQ) < 2e-6;
        g.setColour (isBar ? Colours::gold.withAlpha (0.55f) : juce::Colour (0x18ffffff));
        g.drawVerticalLine ((int) x, area.getY(), area.getBottom());
    }

    const float cellW = juce::jlimit (2.0f, 9.0f, area.getWidth() * (float) (0.22 / len));
    for (auto& e : seq.events)
    {
        auto it = std::find (rows.begin(), rows.end(), e.piece);
        if (it == rows.end()) continue;
        const float y = area.getY() + rowH * (float) std::distance (rows.begin(), it);
        double t = e.ppq + (bpm ? e.ms * bpm() / 60000.0 : 0.0);
        t = std::fmod (t, len); if (t < 0) t += len;
        const float x = area.getX() + (float) (t / len) * area.getWidth();
        const float vn = (float) e.vel / 127.0f;
        const float h = juce::jmax (2.0f, (rowH - 2.0f) * (0.15f + 0.85f * vn));
        g.setColour (pieceColour (e.piece).withAlpha (0.25f + 0.75f * vn));
        g.fillRoundedRectangle (x, y + (rowH - h) * 0.5f, cellW, h, 1.5f);
    }

    if (playhead)
    {
        const double ph = playhead();
        if (ph >= 0.0)
        {
            const float x = area.getX() + (float) ph * area.getWidth();
            g.setColour (juce::Colours::white.withAlpha (0.85f));
            g.drawLine (x, area.getY(), x, area.getBottom(), 1.5f);
        }
    }

    if (! compact && title.isNotEmpty())
    {
        g.setColour (Colours::textDim);
        g.setFont (uiFont (11.5f));
        g.drawText (title, getLocalBounds().reduced (10, 4), juce::Justification::bottomRight);
    }
}

// ============================================================ PresetStrip
int PresetStrip::indexAt (int x) const
{
    const int n = perPage;
    return juce::jlimit (0, n - 1, x * n / juce::jmax (1, getWidth()));
}

void PresetStrip::paint (juce::Graphics& g)
{
    if (! names) return;
    auto list = names();
    const int n = perPage;
    const int first = firstNumber ? firstNumber() : 0;
    const float w = (float) getWidth() / (float) n;
    const int sel = selected ? selected() : -1;

    for (int i = 0; i < n; ++i)
    {
        auto r = juce::Rectangle<float> (w * (float) i, 0.0f, w, (float) getHeight()).reduced (2.0f, 1.0f);
        const bool on = (i == sel);
        g.setColour (on ? juce::Colour (0xff3a2d18) : (i == hoverIdx ? juce::Colour (0xff26221d) : juce::Colour (0xff17140f)));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (on ? Colours::gold : Colours::panelLine);
        g.drawRoundedRectangle (r, 6.0f, on ? 1.6f : 1.0f);

        drawStar (g, juce::Rectangle<float> (r.getRight() - 22.0f, r.getY() + 4.0f, 16.0f, 16.0f), isFav && isFav (i));
        auto tr = r.reduced (8.0f, 4.0f);
        g.setColour (Colours::gold);
        g.setFont (uiFont (15.0f, true));
        g.setFont (uiFont (13.0f, true));
        g.drawText (juce::String (first + i + 1).paddedLeft ('0', 2), tr.removeFromTop (15.0f), juce::Justification::topLeft);
        g.setColour (on ? juce::Colours::white : Colours::cream);
        g.setFont (uiFont (10.5f, on));
        g.drawFittedText (list[i], tr.toNearestInt(), juce::Justification::topLeft, 2, 0.85f);
    }
}

void PresetStrip::mouseDown (const juce::MouseEvent& e)
{
    const int i = indexAt (e.x);
    const float w = (float) getWidth() / (float) perPage;
    if (e.position.x > w * (float) (i + 1) - 26.0f && e.position.y < 24.0f) { if (onStar) onStar (i); repaint(); return; }
    if (onSelect) onSelect (i);
}

void may::drawStar (juce::Graphics& g, juce::Rectangle<float> r, bool on)
{
    juce::Path p;
    p.addStar (r.getCentre(), 5, r.getWidth() * 0.22f, r.getWidth() * 0.5f, 0.0f);
    if (on) { g.setColour (Colours::gold); g.fillPath (p); }
    else    { g.setColour (juce::Colour (0xff6a6358)); g.strokePath (p, juce::PathStrokeType (1.4f)); }
}

// ============================================================ GenreList
void GenreList::paint (juce::Graphics& g)
{
    const float h = 30.0f;
    for (size_t i = 0; i < items.size(); ++i)
    {
        const auto& it = items[i];
        auto r = juce::Rectangle<float> (0.0f, h * (float) i, (float) getWidth(), h - 2.0f);
        if (it.header)
        {
            g.setColour (Colours::gold);
            g.setFont (uiFont (11.0f, true));
            g.drawText (it.name, r.reduced (10.0f, 0.0f), juce::Justification::bottomLeft);
            continue;
        }
        const bool on = it.id == selected && it.sub == selectedSub;
        if (on)
        {
            g.setColour (juce::Colour (0xff3a2d18)); g.fillRoundedRectangle (r, 5.0f);
            g.setColour (Colours::gold); g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);
        }
        auto tr = r.reduced (10.0f, 0.0f);
        if (it.sub >= 0) tr.removeFromLeft (14.0f);
        if (it.id == -2 && it.sub < 0) { drawStar (g, tr.removeFromLeft (16.0f).withSizeKeepingCentre (14.0f, 14.0f), true); tr.removeFromLeft (6.0f); }
        g.setColour (on ? juce::Colours::white : Colours::cream);
        g.setFont (uiFont (it.sub >= 0 ? 13.0f : 14.0f, it.sub < 0));
        g.drawText (it.name, tr, juce::Justification::centredLeft);
        g.setColour (Colours::textDim);
        g.setFont (uiFont (12.0f));
        g.drawText (juce::String (it.count), tr, juce::Justification::centredRight);
    }
}

void GenreList::mouseDown (const juce::MouseEvent& e)
{
    const int i = (int) (e.position.y / 30.0f);
    if (i >= 0 && i < (int) items.size() && ! items[(size_t) i].header && onSelect)
        onSelect (items[(size_t) i].id, items[(size_t) i].sub);
}

// ============================================================ XYPad
void XYPad::setFrom (juce::Point<float> pt)
{
    auto a = padArea();
    const float x = juce::jlimit (0.0f, 1.0f, (pt.x - a.getX()) / a.getWidth());
    const float y = juce::jlimit (0.0f, 1.0f, 1.0f - (pt.y - a.getY()) / a.getHeight());
    proc.setParam ("mdX", x);
    proc.setParam ("mdY", y);
    repaint();
}

void XYPad::mouseDown (const juce::MouseEvent& e)
{
    active = true;
    if (auto* p = proc.apvts.getParameter ("mdX")) p->beginChangeGesture();
    if (auto* p = proc.apvts.getParameter ("mdY")) p->beginChangeGesture();
    setFrom (e.position);
}

void XYPad::mouseDrag (const juce::MouseEvent& e) { setFrom (e.position); }

void XYPad::mouseUp (const juce::MouseEvent&)
{
    active = false;
    if (auto* p = proc.apvts.getParameter ("mdX")) p->endChangeGesture();
    if (auto* p = proc.apvts.getParameter ("mdY")) p->endChangeGesture();
    repaint();
    if (onRelease) onRelease();
}

void XYPad::paint (juce::Graphics& g)
{
    auto a = padArea();
    juce::ColourGradient gr (juce::Colour (0xff5a2a18), a.getX(), a.getY(), juce::Colour (0xff15130f), a.getX(), a.getBottom(), false);
    g.setGradientFill (gr);
    g.fillRoundedRectangle (a, 6.0f);
    juce::ColourGradient gx (juce::Colour (0x00000000), a.getX(), 0, juce::Colour (0x40d8a94e), a.getRight(), 0, false);
    g.setGradientFill (gx);
    g.fillRoundedRectangle (a, 6.0f);

    g.setColour (juce::Colour (0x22ffffff));
    for (int i = 1; i < 4; ++i)
    {
        g.drawVerticalLine ((int) (a.getX() + a.getWidth() * (float) i / 4.0f), a.getY(), a.getBottom());
        g.drawHorizontalLine ((int) (a.getY() + a.getHeight() * (float) i / 4.0f), a.getX(), a.getRight());
    }
    g.setColour (Colours::gold.withAlpha (0.7f));
    g.drawRoundedRectangle (a, 6.0f, 1.2f);

    g.setColour (Colours::textDim);
    g.setFont (uiFont (11.0f, true));
    auto full = getLocalBounds().toFloat();
    g.drawText ("FORTE", full.withHeight (18.0f), juce::Justification::centred);
    g.drawText ("LEGGERO", full.withTop (full.getBottom() - 18.0f), juce::Justification::centred);
    g.saveState();
    g.addTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi, 11.0f, full.getCentreY()));
    g.drawText ("DRITTO", juce::Rectangle<float> (11.0f - 60.0f, full.getCentreY() - 8.0f, 120.0f, 16.0f), juce::Justification::centred);
    g.restoreState();
    g.saveState();
    g.addTransform (juce::AffineTransform::rotation (juce::MathConstants<float>::halfPi, full.getRight() - 11.0f, full.getCentreY()));
    g.drawText ("IMPREVEDIBILE", juce::Rectangle<float> (full.getRight() - 11.0f - 60.0f, full.getCentreY() - 8.0f, 120.0f, 16.0f), juce::Justification::centred);
    g.restoreState();

    const float x = proc.getParam ("mdX"), y = proc.getParam ("mdY");
    const juce::Point<float> dot (a.getX() + x * a.getWidth(), a.getY() + (1.0f - y) * a.getHeight());
    g.setColour (Colours::gold.withAlpha (0.25f));
    g.drawHorizontalLine ((int) dot.y, a.getX(), a.getRight());
    g.drawVerticalLine ((int) dot.x, a.getY(), a.getBottom());
    const float rad = active ? 11.0f : 9.0f;
    g.setColour (Colours::gold.withAlpha (0.3f));
    g.fillEllipse (juce::Rectangle<float> (rad * 3.2f, rad * 3.2f).withCentre (dot));
    juce::ColourGradient dg (juce::Colour (0xfffff0c0), dot.x - rad * 0.4f, dot.y - rad * 0.5f, juce::Colour (0xffa87a2c), dot.x + rad, dot.y + rad, true);
    g.setGradientFill (dg);
    g.fillEllipse (juce::Rectangle<float> (rad * 2.0f, rad * 2.0f).withCentre (dot));

    g.setColour (Colours::cream);
    g.setFont (uiFont (11.5f));
    g.drawText ("Forza " + juce::String (juce::roundToInt (y * 100)) + "%   " + juce::String::fromUTF8 ("Imprevedibilità") + " " + juce::String (juce::roundToInt (x * 100)) + "%",
                a.reduced (6.0f).withTop (a.getBottom() - 20.0f), juce::Justification::centredLeft);
}

// ============================================================ Editor
MayDrummerEditor::MayDrummerEditor (MayDrummerProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);
    logo = juce::ImageCache::getFromMemory (BinaryData::logo_jpg, BinaryData::logo_jpgSize);

    addAndMakeVisible (content);
    for (auto* t : { &tabDrummer, &tabBrowser, &tabSong, &tabMap })
    {
        content.addAndMakeVisible (*t);
        t->setClickingTogglesState (false);
    }
    tabDrummer.onClick = [this] { showPage (0); };
    tabSong.onClick = [this] { showPage (1); };
    tabMap.onClick = [this] { showPage (2); };
    tabBrowser.onClick = [this] { showPage (3); };

    content.addAndMakeVisible (saveBtn);
    content.addAndMakeVisible (loadBtn);
    saveBtn.setBounds (326, 26, 86, 30);
    loadBtn.setBounds (326, 60, 86, 30);
    saveBtn.setTooltip ("Salva tutte le impostazioni attuali con un nome");
    loadBtn.setTooltip ("Richiama un'impostazione salvata");
    saveBtn.onClick = [this] { askSaveName(); };
    loadBtn.onClick = [this] { showLoadMenu(); };

    for (auto* pg : { &drummerPage, &songPage, &mapPage, &browserPage })
    {
        content.addChildComponent (*pg);
        pg->setBounds (12, 118, 1156, 836);
    }

    buildDrummerPage();
    buildSongPage();
    buildMapPage();
    buildBrowserPage();
    showPage (0);

    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio ((double) designW / designH);
    setResizeLimits (designW / 2, designH / 2, designW * 3 / 2, designH * 3 / 2);
    addAndMakeVisible (donateBtn);
    addAndMakeVisible (helpBtn);
    addAndMakeVisible (footer);
    donateBtn.onClick = [this] { welcome.open(); };
    helpBtn.onClick = [this]
    {
        may::showHelpMenu (helpBtn, may::Welcome::storedEnglish(), "May Studio Midi Drummer", JucePlugin_VersionString,
                           juce::PluginHostType().getHostDescription(), {}, [this] { welcome.open(); });
    };
    welcome.onLanguageChanged = [this] { donateBtn.repaint(); };
    addChildComponent (welcome);

    setSize ((int) (designW * 0.82), (int) (designH * 0.82));
    welcome.showIfFirstTime();
    startTimerHz (30);
}

MayDrummerEditor::~MayDrummerEditor()
{
    stopTimer();
    if (saveDialog != nullptr) { saveDialog->exitModalState (0); saveDialog->setLookAndFeel (nullptr); saveDialog.reset(); }
    setLookAndFeel (nullptr);
}

void MayDrummerEditor::resized()
{
    const float scale = (float) getWidth() / (float) designW;
    content.setBounds (0, 0, designW, designH);
    content.setTransform (juce::AffineTransform::scale (scale));
    donateBtn.setBounds (getWidth() - may::DonateButton::kWidth - 26, getHeight() - may::DonateButton::kHeight - 6,
                         may::DonateButton::kWidth, may::DonateButton::kHeight);
    helpBtn.setBounds (donateBtn.getX() - may::HelpButton::kSize - 6, donateBtn.getY(), may::HelpButton::kSize, may::HelpButton::kSize);
    footer.setBounds (10, getHeight() - may::FooterLabel::kHeight - 8, may::FooterLabel::kWidth, may::FooterLabel::kHeight);
    welcome.setBounds (getLocalBounds());
}

void MayDrummerEditor::showPage (int p)
{
    page = p;
    drummerPage.setVisible (p == 0);
    songPage.setVisible (p == 1);
    mapPage.setVisible (p == 2);
    browserPage.setVisible (p == 3);
    tabBrowser.setToggleState (p == 3, juce::dontSendNotification);
    if (p == 3) rebuildBrowser();
    tabDrummer.setToggleState (p == 0, juce::dontSendNotification);
    tabSong.setToggleState (p == 1, juce::dontSendNotification);
    tabMap.setToggleState (p == 2, juce::dontSendNotification);
    content.repaint();
}

// ---------------------------------------------------------------- helper
juce::ComboBox& MayDrummerEditor::combo (const juce::String& id, juce::Component& parent)
{
    auto c = std::make_unique<juce::ComboBox>();
    if (auto* ch = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
        c->addItemList (ch->choices, 1);
    parent.addAndMakeVisible (*c);
    comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, id, *c));
    byId[id] = c.get();
    combos.push_back (std::move (c));
    return *combos.back();
}

juce::ToggleButton& MayDrummerEditor::toggle (const juce::String& id, const juce::String& text, juce::Component& parent)
{
    auto t = std::make_unique<juce::ToggleButton> (text);
    parent.addAndMakeVisible (*t);
    toggleAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, *t));
    byId[id] = t.get();
    toggles.push_back (std::move (t));
    return *toggles.back();
}

Knob& MayDrummerEditor::knob (const juce::String& id, const juce::String& text, juce::Component& parent)
{
    auto k = std::make_unique<Knob>();
    parent.addAndMakeVisible (k->slider);
    parent.addAndMakeVisible (k->label);
    k->label.setText (text, juce::dontSendNotification);
    k->label.setJustificationType (juce::Justification::centred);
    k->label.setFont (uiFont (12.0f, true));
    k->label.setColour (juce::Label::textColourId, Colours::cream);
    k->id = id;
    parent.addAndMakeVisible (k->value);
    k->value.setJustificationType (juce::Justification::centred);
    k->value.setFont (uiFont (10.5f));
    k->value.setColour (juce::Label::textColourId, Colours::textDim);
    k->value.setInterceptsMouseClicks (false, false);
    k->att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k->slider);
    knobs.push_back (std::move (k));
    return *knobs.back();
}

juce::Label& MayDrummerEditor::caption (const juce::String& text, juce::Component& parent)
{
    auto l = std::make_unique<juce::Label> (juce::String(), text);
    l->setFont (uiFont (12.5f, true));
    l->setColour (juce::Label::textColourId, Colours::gold);
    l->setInterceptsMouseClicks (false, false);
    parent.addAndMakeVisible (*l);
    captions.push_back (std::move (l));
    return *captions.back();
}

// ---------------------------------------------------------------- Pagina DRUMMER
void MayDrummerEditor::buildDrummerPage()
{
    auto& pg = drummerPage;

    caption ("1  GENERE", pg).setBounds (20, 14, 200, 18);
    genreBox = &combo ("genre", pg);
    genreBox->setBounds (24, 38, 200, 30);

    caption ("2  SOTTOGENERE", pg).setBounds (236, 14, 220, 18);
    pg.addAndMakeVisible (subBox);
    subBox.setBounds (240, 38, 220, 30);
    subBox.onChange = [this] { if (subBox.getSelectedItemIndex() >= 0) proc.setParam ("sub", (float) subBox.getSelectedItemIndex()); };

    caption ("3  MISURA", pg).setBounds (472, 14, 120, 18);
    combo ("meter", pg).setBounds (476, 38, 110, 30);
    toggle ("meterHost", "Da progetto", pg).setBounds (476, 72, 120, 22);

    caption ("BPM", pg).setBounds (602, 14, 160, 18);
    pg.addAndMakeVisible (bpmSlider);
    bpmSlider.setBounds (606, 38, 150, 30);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 150, 30);
    bpmAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "bpm", bpmSlider);
    bpmSlider.setTextValueSuffix (" BPM");
    toggle ("bpmHost", "Da progetto", pg).setBounds (606, 72, 120, 22);

    caption ("BATTUTE", pg).setBounds (768, 14, 90, 18);
    combo ("bars", pg).setBounds (772, 38, 80, 30);

    caption ("FEEL", pg).setBounds (864, 14, 110, 18);
    combo ("feelMode", pg).setBounds (868, 38, 110, 30);

    pg.addAndMakeVisible (diceBtn);
    pg.addAndMakeVisible (diceGenreBtn);
    diceBtn.setBounds (996, 30, 138, 28);
    diceGenreBtn.setBounds (996, 64, 138, 28);
    diceBtn.setTooltip ("Sottogenere, preset e variazione casuali nel genere scelto");
    diceBtn.onClick = [this] { proc.randomise (false); };
    diceGenreBtn.onClick = [this] { proc.randomise (true); };

    // 4 Preset
    pg.addAndMakeVisible (prevBtn);
    pg.addAndMakeVisible (nextBtn);
    pg.addAndMakeVisible (presets);
    prevBtn.setBounds (24, 144, 30, 42);
    nextBtn.setBounds (1102, 144, 30, 42);
    presets.setBounds (58, 142, 1040, 46);
    presets.names = [this]
    {
        juce::StringArray a;
        for (int i = 0; i < PresetStrip::perPage; ++i)
            a.add (PatternLibrary::get().getPattern ((int) proc.getParam ("genre"), (int) proc.getParam ("sub"), uiPage * PresetStrip::perPage + i).label);
        return a;
    };
    presets.firstNumber = [this] { return uiPage * PresetStrip::perPage; };

    pg.addAndMakeVisible (pagePrev);
    pg.addAndMakeVisible (pageNext);
    pg.addAndMakeVisible (pageLabel);
    caption ("PAGINA", pg).setBounds (560, 116, 60, 20);
    pagePrev.setBounds (622, 116, 26, 22);
    pageLabel.setBounds (650, 116, 70, 22);
    pageNext.setBounds (722, 116, 26, 22);
    pageLabel.setJustificationType (juce::Justification::centred);
    pageLabel.setFont (uiFont (12.5f, true));
    pageLabel.setColour (juce::Label::textColourId, Colours::cream);
    const int pages = PatternLibrary::presetsPerSub / PresetStrip::perPage;
    pagePrev.onClick = [this, pages] { uiPage = (uiPage + pages - 1) % pages; presets.repaint(); };
    pageNext.onClick = [this, pages] { uiPage = (uiPage + 1) % pages; presets.repaint(); };

    pg.addAndMakeVisible (presetInfo);
    presetInfo.setBounds (196, 116, 360, 20);
    presetInfo.setFont (uiFont (12.0f));
    presetInfo.setColour (juce::Label::textColourId, Colours::textDim);
    pg.addAndMakeVisible (quickSearch);
    quickSearch.setBounds (820, 116, 312, 24);
    quickSearch.setTextToShowWhenEmpty ("Cerca tra 16000 preset (es. rock shuffle leggero)", Colours::textDim);
    quickSearch.setColour (juce::TextEditor::backgroundColourId, Colours::lcd);
    quickSearch.setColour (juce::TextEditor::textColourId, Colours::cream);
    quickSearch.setColour (juce::TextEditor::outlineColourId, Colours::panelLine);
    quickSearch.setColour (juce::TextEditor::focusedOutlineColourId, Colours::gold);
    quickSearch.setFont (uiFont (13.0f));
    quickSearch.setIndents (8, 4);
    auto openSearch = [this]
    {
        searchBox.setText (quickSearch.getText(), false);
        browserGenre = -1; browserSub = -1;
        showPage (3);
        searchBox.grabKeyboardFocus();
        searchBox.moveCaretToEnd();
    };
    quickSearch.onReturnKey = openSearch;
    quickSearch.onTextChange = [this, openSearch] { if (quickSearch.getText().length() >= 3) openSearch(); };
    presets.selected = [this] { return (int) proc.getParam ("preset") - uiPage * PresetStrip::perPage; };
    presets.onSelect = [this] (int i) { proc.setParam ("preset", (float) (uiPage * PresetStrip::perPage + i)); };
    presets.isFav = [this] (int i) { return proc.isFavourite ((int) proc.getParam ("genre"), (int) proc.getParam ("sub"), uiPage * PresetStrip::perPage + i); };
    presets.onStar = [this] (int i) { proc.toggleFavourite ((int) proc.getParam ("genre"), (int) proc.getParam ("sub"), uiPage * PresetStrip::perPage + i); };
    auto step = [this] (int d)
    {
        int idx = ((int) proc.getParam ("sub")) * PatternLibrary::presetsPerSub + (int) proc.getParam ("preset") + d;
        const int total = PatternLibrary::subsPerStyle * PatternLibrary::presetsPerSub;
        idx = (idx + total) % total;
        proc.setParam ("sub", (float) (idx / PatternLibrary::presetsPerSub));
        proc.setParam ("preset", (float) (idx % PatternLibrary::presetsPerSub));
    };
    prevBtn.onClick = [step] { step (-1); };
    nextBtn.onClick = [step] { step (1); };

    // 5 Main / Riff / Finale
    for (int i = 0; i < NumParts; ++i)
    {
        auto& b = partBtns[(size_t) i];
        b.setButtonText (juce::String (partName (i)).toUpperCase());
        pg.addAndMakeVisible (b);
        b.setBounds (24 + i * 102, 236, 96, 28);
        b.onClick = [this, i] { proc.setParam ("part", (float) i); proc.setParam ("playMode", 0.0f); };
    }
    pg.addAndMakeVisible (mainView);
    mainView.setBounds (24, 272, 816, 160);
    addInstrumentRow (pg, 24, 438, 816, false);
    mainView.source = [this] () -> const DrumSequence& { return proc.partSeqs[(size_t) juce::jlimit (0, NumParts - 1, (int) proc.getParam ("part"))]; };
    mainView.playhead = [this] { return proc.getParam ("playMode") < 0.5f ? proc.playPosition.load() : -1.0; };
    mainView.bpm = [this] { return proc.effectiveBpm(); };
    mainView.noteOf = [this] (int p) { return proc.getNote (p); };
    mainView.makeFiles = [this] { return juce::StringArray { proc.writePartFile ((int) proc.getParam ("part")).getFullPathName() }; };

    pg.addAndMakeVisible (dragPart);
    dragPart.setBounds (856, 236, 276, 86);
    dragPart.makeFiles = mainView.makeFiles;
    pg.addAndMakeVisible (dragAll);
    dragAll.setBounds (856, 328, 276, 52);
    dragAll.makeFiles = [this] { return proc.writeAllPartFiles(); };

    pg.addAndMakeVisible (previewBtn);
    previewBtn.setBounds (856, 386, 132, 26);
    previewBtn.setClickingTogglesState (true);
    toggleAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "preview", previewBtn));
    combo ("playMode", pg).setBounds (998, 386, 134, 26);
    toggle ("kitOn", "Suoni interni", pg).setBounds (856, 416, 130, 24);
    pg.addAndMakeVisible (kitVolSlider);
    kitVolSlider.setBounds (998, 417, 134, 22);
    kitVolSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 134, 22);
    kitVolAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "kitVol", kitVolSlider);
    kitVolSlider.textFromValueFunction = [] (double v) { return "Vol kit " + juce::String (juce::roundToInt (v)) + "%"; };
    kitVolSlider.updateText();
    previewBtn.setTooltip ("Invia il MIDI anche a trasporto fermo");
    pg.addAndMakeVisible (shuffleBtn);
    pg.addAndMakeVisible (originalBtn);
    shuffleBtn.setBounds (856, 444, 180, 26);
    originalBtn.setBounds (1042, 444, 90, 26);
    shuffleBtn.onClick = [this] { proc.setParam ("alt", (float) (1 + juce::Random::getSystemRandom().nextInt (99998))); };
    originalBtn.onClick = [this] { proc.setParam ("alt", 0.0f); };

    // Humanize e suono
    struct KD { const char* id; const char* name; };
    const KD kd[] = { { "humTiming", "TIMING" }, { "humVel", "VELOCITY" }, { "volume", "VOLUME" }, { "feel", "PUSH/LAID" },
                      { "ghost", "GHOST" }, { "hatAccent", "ACCENTI" }, { "swing", "SWING" } };
    for (int i = 0; i < 7; ++i)
    {
        auto& k = knob (kd[i].id, kd[i].name, pg);
        k.slider.setBounds (30 + i * 80, 504, 56, 56);
        k.label.setBounds (18 + i * 80, 560, 80, 16);
        k.value.setBounds (18 + i * 80, 575, 80, 14);
    }
    caption ("TEMPO", pg).setBounds (588, 506, 110, 18);
    combo ("timeFeel", pg).setBounds (592, 530, 104, 30);
    caption ("PIATTI", pg).setBounds (700, 506, 90, 18);
    combo ("cymbals", pg).setBounds (704, 530, 84, 30);
    caption ("SWING SU", pg).setBounds (792, 506, 110, 18);
    combo ("swing16", pg).setBounds (796, 530, 104, 30);
    caption ("RULLANTE", pg).setBounds (904, 506, 130, 18);
    combo ("sideStick", pg).setBounds (908, 530, 124, 30);
    toggle ("crash", "Crash su 1", pg).setBounds (1040, 533, 104, 24);

    // 6 May Drummer
    pg.addAndMakeVisible (pad);
    pad.setBounds (24, 628, 260, 196);
    pad.onRelease = [this] { proc.regenerateNow(); snapshotPreviews(); };

    pg.addAndMakeVisible (lvUp);
    pg.addAndMakeVisible (lvDn);
    pg.addAndMakeVisible (lvLabel);
    lvUp.setBounds (296, 636, 56, 44);
    lvDn.setBounds (296, 742, 56, 44);
    lvLabel.setBounds (296, 684, 56, 54);
    lvLabel.setJustificationType (juce::Justification::centred);
    lvLabel.setColour (juce::Label::backgroundColourId, Colours::lcd);
    lvLabel.setColour (juce::Label::outlineColourId, Colours::panelLine);
    lvLabel.setColour (juce::Label::textColourId, Colours::gold);
    lvLabel.setFont (uiFont (10.5f, true));
    lvLabel.setMinimumHorizontalScale (0.55f);
    lvUp.setTooltip ("Rendi la traccia piu complessa");
    lvDn.setTooltip ("Semplifica la traccia");
    lvUp.onClick = [this] { proc.setParam ("level", juce::jmin (5.0f, proc.getParam ("level") + 1.0f)); };
    lvDn.onClick = [this] { proc.setParam ("level", juce::jmax (-5.0f, proc.getParam ("level") - 1.0f)); };
    auto& lvHint = caption ("arricchisci / semplifica", pg);
    lvHint.setBounds (286, 790, 76, 26);
    lvHint.setFont (uiFont (9.5f));
    lvHint.setColour (juce::Label::textColourId, Colours::textDim);
    lvHint.setJustificationType (juce::Justification::centred);
    lvHint.setMinimumHorizontalScale (1.0f);

    const char* titles[] = { "MAIN", "RIFF", "FINALE" };
    const int parts[] = { PMain, PRiff, PEnd1 };
    for (int i = 0; i < 3; ++i)
    {
        auto& v = mdPreview[(size_t) i];
        v.compact = true;
        v.title = titles[i];
        v.source = [this, i] () -> const DrumSequence& { return mdSeqs[(size_t) i]; };
        const int part = parts[i];
        v.makeFiles = [this, part] { return juce::StringArray { proc.writePartFile (part).getFullPathName() }; };
        pg.addAndMakeVisible (v);
        v.setBounds (366, 632 + i * 64, 766, 58);
        v.bpm = [this] { return proc.effectiveBpm(); };
    }
    pg.addAndMakeVisible (mdHint);
    mdHint.setBounds (366, 606, 766, 22);
    mdHint.setJustificationType (juce::Justification::centredRight);
    mdHint.setFont (uiFont (12.0f));
    mdHint.setColour (juce::Label::textColourId, Colours::textDim);
    mdHint.setText ("Sposta il puntino e rilascialo: a destra l'anteprima. Ogni riga si trascina nella DAW.", juce::dontSendNotification);

    refreshSubCombo();
    snapshotPreviews();
}

void MayDrummerEditor::addInstrumentRow (juce::Component& parent, int x, int y, int w, bool song)
{
    auto& cap = caption ("SINGOLI STRUMENTI", parent);
    cap.setBounds (x - 4, y + 4, 130, 20);
    cap.setFont (uiFont (11.0f, true));
    const int startX = x + 126;
    const int n = NumGroups + 1;
    const int cw = (w - 126 - (n - 1) * 5) / n;
    for (int i = 0; i < n; ++i)
    {
        const bool all = (i == NumGroups);
        static const char* shortNames[] = { "CASSA", "RULLANTE", "HI-HAT", "TOM", "PIATTI", "PERC." };
        auto z = std::make_unique<DragZone> (all ? "SEPARATI" : shortNames[i], "");
        const int group = all ? -1 : i;
        z->makeFiles = [this, song, group] { return proc.writeInstrumentFiles (song, group); };
        z->setTooltip (all ? "Un file MIDI per ogni strumento" : "Solo " + juce::String (groupName (i)).toLowerCase());
        parent.addAndMakeVisible (*z);
        z->setBounds (startX + i * (cw + 5), y, cw, 26);
        instrumentZones.push_back (std::move (z));
    }
}

void MayDrummerEditor::snapshotPreviews()
{
    mdSeqs[0] = proc.partSeqs[PMain];
    mdSeqs[1] = proc.partSeqs[PRiff];
    mdSeqs[2] = proc.partSeqs[PEnd1];
    for (auto& v : mdPreview) v.repaint();
}

void MayDrummerEditor::refreshSubCombo()
{
    const int g = (int) proc.getParam ("genre");
    if (g != lastGenre)
    {
        subBox.clear (juce::dontSendNotification);
        subBox.addItemList (PatternLibrary::get().subNames (g), 1);
        lastGenre = g;
    }
    subBox.setSelectedItemIndex ((int) proc.getParam ("sub"), juce::dontSendNotification);
}

// ---------------------------------------------------------------- Pagina SONG
void MayDrummerEditor::buildSongPage()
{
    auto& pg = songPage;
    caption ("BATTUTE TOTALI", pg).setBounds (20, 36, 200, 18);
    pg.addAndMakeVisible (songBarsSlider);
    songBarsSlider.setBounds (24, 60, 200, 30);
    songBarsSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 200, 30);
    songBarsSlider.setTextValueSuffix (" battute");
    songBarsAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "songBars", songBarsSlider);

    caption ("SEZIONE (BATTUTE)", pg).setBounds (240, 36, 150, 18);
    combo ("sectionBars", pg).setBounds (244, 60, 130, 30);
    caption ("FILL", pg).setBounds (390, 36, 130, 18);
    combo ("fillEvery", pg).setBounds (394, 60, 130, 30);
    caption ("FINALE", pg).setBounds (540, 36, 140, 18);
    combo ("ending", pg).setBounds (544, 60, 140, 30);
    toggle ("countIn", "Count-in di bacchette", pg).setBounds (704, 56, 200, 24);
    toggle ("altRiff", "Alterna Main e Riff", pg).setBounds (704, 86, 200, 24);
    caption ("RIPRODUZIONE", pg).setBounds (930, 36, 160, 18);
    combo ("playMode", pg).setBounds (934, 60, 190, 30);

    auto& hint = caption ("Le sezioni alternano Main (marrone) e Riff (oro); i fill (rosso) chiudono le frasi, il finale (grigio) chiude il brano.", pg);
    hint.setBounds (20, 108, 1100, 20);
    hint.setColour (juce::Label::textColourId, Colours::textDim);
    hint.setFont (uiFont (12.0f));

    pg.addAndMakeVisible (songView);
    songView.setBounds (24, 184, 1108, 380);
    songView.source = [this] () -> const DrumSequence& { return proc.songSeq; };
    songView.playhead = [this] { return proc.getParam ("playMode") > 0.5f ? proc.playPosition.load() : -1.0; };
    songView.bpm = [this] { return proc.effectiveBpm(); };
    songView.noteOf = [this] (int p) { return proc.getNote (p); };
    songView.makeFiles = [this] { return juce::StringArray { proc.writeSongFile().getFullPathName() }; };

    pg.addAndMakeVisible (dragSong);
    dragSong.setBounds (24, 620, 520, 120);
    dragSong.makeFiles = songView.makeFiles;
    addInstrumentRow (pg, 24, 760, 1108, true);

    auto& t = caption ("Trascina il pulsante dorato (o direttamente l'anteprima) su una traccia MIDI o strumento della DAW. "
                       "Il file contiene tempo e misura del progetto.", pg);
    t.setBounds (566, 630, 560, 60);
    t.setColour (juce::Label::textColourId, Colours::cream);
    t.setFont (uiFont (13.0f));
    t.setJustificationType (juce::Justification::topLeft);
    t.setMinimumHorizontalScale (1.0f);
}

// ---------------------------------------------------------------- Pagina MAPPA
void MayDrummerEditor::buildMapPage()
{
    auto& pg = mapPage;
    for (int i = 0; i < NumPieces; ++i)
    {
        const int col = i < 9 ? 0 : 1;
        const int row = col == 0 ? i : i - 9;
        auto& l = noteLabels[(size_t) i];
        l.setText (pieceName (i), juce::dontSendNotification);
        l.setFont (uiFont (14.0f, true));
        l.setJustificationType (juce::Justification::centredRight);
        pg.addAndMakeVisible (l);
        l.setBounds (24 + col * 560, 40 + row * 32, 170, 28);

        auto& s = noteSliders[(size_t) i];
        s.setSliderStyle (juce::Slider::IncDecButtons);
        s.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 110, 28);
        s.setRange (0, 127, 1);
        s.textFromValueFunction = [] (double v) { return juce::String ((int) v) + "  " + juce::MidiMessage::getMidiNoteName ((int) v, true, true, 3); };
        s.valueFromTextFunction = [] (const juce::String& t) { return (double) t.getIntValue(); };
        s.setValue (proc.getNote (i), juce::dontSendNotification);
        s.onValueChange = [this, i] { proc.setNote (i, (int) noteSliders[(size_t) i].getValue()); };
        pg.addAndMakeVisible (s);
        s.setBounds (204 + col * 560, 40 + row * 32, 190, 28);
    }

    pg.addAndMakeVisible (resetMapBtn);
    resetMapBtn.setBounds (24, 380, 220, 30);
    resetMapBtn.onClick = [this]
    {
        proc.resetNoteMap();
        for (int i = 0; i < NumPieces; ++i) noteSliders[(size_t) i].setValue (proc.getNote (i), juce::dontSendNotification);
    };

    caption ("CANALE MIDI", pg).setBounds (270, 360, 140, 18);
    pg.addAndMakeVisible (channelSlider);
    channelSlider.setBounds (274, 382, 150, 28);
    channelSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 60, 28);
    channelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "channel", channelSlider);

    toggle ("followHost", "Segui il trasporto della DAW", pg).setBounds (450, 384, 260, 24);

    caption ("MUTE DEL KIT (vale per riproduzione e file trascinati)", pg).setBounds (20, 430, 600, 18);
    const char* names[] = { "Cassa", "Rullante", "Hi-Hat", "Tom", "Piatti", "Percussioni" };
    for (int i = 0; i < NumGroups; ++i)
        toggle ("mute" + juce::String (i), names[i], pg).setBounds (24 + i * 150, 454, 140, 24);

    pg.addAndMakeVisible (mapHelp);
    mapHelp.setBounds (24, 500, 1100, 300);
    mapHelp.setJustificationType (juce::Justification::topLeft);
    mapHelp.setFont (uiFont (14.0f));
    mapHelp.setColour (juce::Label::textColourId, Colours::cream);
    mapHelp.setText (juce::String::fromUTF8 ("La mappa predefinita è General MIDI: funziona con Addictive Drums 2, EZdrummer, Superior Drummer, "
                     "BFD, Steven Slate Drums e i kit delle DAW impostati in modalità GM. Se il tuo strumento usa note diverse, "
                     "cambiale qui: valgono sia per la riproduzione sia per i file trascinati.\n\n"
                     "KEYSWITCH dal vivo: le note da C-2 a G-2 (0-7) scelgono la parte mentre la DAW suona: "
                     "C-2 Main, C#-2 Riff, D-2 Fill 1, D#-2 Fill 2, E-2 Fill 3, F-2 Finale 1, F#-2 Finale 2, G-2 Finale 3.\n\n"
                     "USO DAL VIVO: metti May Studio Midi Drummer su una traccia strumento e manda la sua uscita MIDI "
                     "alla traccia del tuo strumento di batteria (Cubase, Reaper, Studio One, Bitwig). "
                     "In alternativa trascina le parti come clip MIDI: funziona in qualunque DAW."),
                     juce::dontSendNotification);
}

// ---------------------------------------------------------------- Disegno
void MayDrummerEditor::paintStudio (juce::Graphics& g, juce::Rectangle<int> r)
{
    auto rf = r.toFloat();
    // parete a doghe di legno
    juce::ColourGradient wall (juce::Colour (0xff4a2f1c), rf.getX(), rf.getY(), juce::Colour (0xff24170e), rf.getX(), rf.getY() + rf.getHeight() * 0.72f, false);
    g.setGradientFill (wall);
    g.fillRect (rf);
    juce::Random rnd (7);
    for (float x = rf.getX(); x < rf.getRight(); x += 22.0f)
    {
        g.setColour (juce::Colour (0x18000000).withAlpha (0.08f + 0.12f * rnd.nextFloat()));
        g.fillRect (x, rf.getY(), 11.0f, rf.getHeight() * 0.72f);
        g.setColour (juce::Colour (0x12ffd9a0));
        g.drawVerticalLine ((int) x, rf.getY(), rf.getY() + rf.getHeight() * 0.72f);
    }
    // pavimento
    auto floor = rf.withTop (rf.getY() + rf.getHeight() * 0.72f);
    juce::ColourGradient fl (juce::Colour (0xff6e6252), floor.getX(), floor.getY(), juce::Colour (0xff2a251f), floor.getX(), floor.getBottom(), false);
    g.setGradientFill (fl);
    g.fillRect (floor);
    // luce calda e vignettatura
    juce::ColourGradient spot (juce::Colour (0x40ffcf8a), rf.getCentreX(), rf.getY() + 80.0f, juce::Colour (0x00000000), rf.getCentreX(), rf.getY() + rf.getHeight() * 0.7f, true);
    g.setGradientFill (spot);
    g.fillRect (rf);
    juce::ColourGradient vig (juce::Colour (0x00000000), rf.getCentreX(), rf.getCentreY(), juce::Colour (0xaa000000), rf.getX(), rf.getY(), true);
    g.setGradientFill (vig);
    g.fillRect (rf);
}

void MayDrummerEditor::paintSection (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& number, const juce::String& title)
{
    auto rf = r.toFloat();
    g.setColour (Colours::panel);
    g.fillRoundedRectangle (rf, 9.0f);
    g.setColour (Colours::panelLine);
    g.drawRoundedRectangle (rf.reduced (0.5f), 9.0f, 1.0f);
    if (title.isEmpty()) return;

    int x = r.getX() + 14;
    if (number.isNotEmpty())
    {
        auto badge = juce::Rectangle<float> ((float) x, (float) r.getY() + 7.0f, 20.0f, 20.0f);
        g.setColour (Colours::gold);
        g.fillEllipse (badge);
        g.setColour (juce::Colour (0xff1a1206));
        g.setFont (uiFont (13.0f, true));
        g.drawText (number, badge, juce::Justification::centred);
        x += 28;
    }
    g.setColour (Colours::gold);
    g.setFont (uiFont (14.0f, true));
    g.drawText (title, x, r.getY() + 6, 600, 22, juce::Justification::centredLeft);
}

void MayDrummerEditor::Content::paint (juce::Graphics& g)
{
    auto& e = ed;
    const auto full = getLocalBounds().toFloat();

    // cornice metallica
    juce::ColourGradient frame (juce::Colour (0xffd6d4cf), 0, 0, juce::Colour (0xff6f6c66), 0, full.getBottom(), false);
    frame.addColour (0.08, juce::Colour (0xffa9a6a0));
    g.setGradientFill (frame);
    g.fillRoundedRectangle (full, 10.0f);

    // barra superiore
    auto bar = juce::Rectangle<float> (6, 6, full.getWidth() - 12, 104);
    juce::ColourGradient bg (juce::Colour (0xff35322e), 0, bar.getY(), juce::Colour (0xff100f0d), 0, bar.getBottom(), false);
    g.setGradientFill (bg);
    g.fillRoundedRectangle (bar, 8.0f);
    g.setColour (juce::Colour (0x40ffffff));
    g.drawHorizontalLine ((int) bar.getY() + 1, bar.getX() + 8, bar.getRight() - 8);

    // display a sinistra
    auto lcd = juce::Rectangle<float> (18, 26, 300, 64);
    g.setColour (Colours::lcd);
    g.fillRoundedRectangle (lcd, 6.0f);
    g.setColour (juce::Colour (0x30ffffff));
    g.drawRoundedRectangle (lcd, 6.0f, 1.0f);
    const auto& pat = e.proc.currentPattern();
    int num = 4, den = 4;
    e.proc.effectiveMeter (num, den);
    g.setColour (Colours::gold);
    g.setFont (uiFont (11.0f, true));
    g.drawFittedText (e.proc.getUserPresetName().isNotEmpty() ? "IMPOSTAZIONE: " + e.proc.getUserPresetName().toUpperCase()
                                                       : juce::String ("MAY STUDIO MIDI DRUMMER"),
                      lcd.reduced (10, 5).removeFromTop (14).toNearestInt(), juce::Justification::centredLeft, 1);
    g.setColour (Colours::cream);
    g.setFont (uiFont (15.0f, true));
    g.drawFittedText (pat.fullName, lcd.reduced (10, 5).withTrimmedTop (14).withHeight (20).toNearestInt(), juce::Justification::centredLeft, 1);
    g.setColour (Colours::textDim);
    g.setFont (uiFont (12.0f));
    g.drawText (juce::String (num) + "/" + juce::String (den) + "   " + juce::String (juce::roundToInt (e.proc.effectiveBpm())) + " BPM   "
                + partName ((int) e.proc.getParam ("part")) + "   " + juce::String (PatternLibrary::get().totalPatterns()) + " pattern",
                lcd.reduced (10, 5).withTrimmedTop (35), juce::Justification::centredLeft);

    // logo al centro, in una "capsula" come nello screenshot
    auto pill = juce::Rectangle<float> (full.getCentreX() - 170.0f, 8.0f, 340.0f, 100.0f);
    juce::ColourGradient ring (juce::Colour (0xffe8e6e1), 0, pill.getY(), juce::Colour (0xff595650), 0, pill.getBottom(), false);
    g.setGradientFill (ring);
    g.fillRoundedRectangle (pill, 50.0f);
    auto inner = pill.reduced (4.0f);
    g.setColour (juce::Colours::black);
    g.fillRoundedRectangle (inner, 46.0f);
    if (e.logo.isValid())
    {
        g.saveState();
        juce::Path clip;
        clip.addRoundedRectangle (inner, 46.0f);
        g.reduceClipRegion (clip);
        g.drawImageWithin (e.logo, (int) inner.getX() + 30, (int) inner.getY() + 3, (int) inner.getWidth() - 60, (int) inner.getHeight() - 6,
                           juce::RectanglePlacement::centred);
        g.restoreState();
    }

    // area principale con lo "studio"
    auto main = juce::Rectangle<int> (12, 118, getWidth() - 24, getHeight() - 130);
    g.saveState();
    juce::Path clip;
    clip.addRoundedRectangle (main.toFloat(), 8.0f);
    g.reduceClipRegion (clip);
    e.paintStudio (g, main);
    for (auto& s : sections)
        if (s.page == e.page)
            e.paintSection (g, s.r.translated (main.getX(), main.getY()), s.num, s.title);
    g.restoreState();
    g.setColour (juce::Colour (0xff1a1816));
    g.drawRoundedRectangle (main.toFloat().expanded (1.0f), 9.0f, 2.0f);
}

// ---------------------------------------------------------------- Pagina BROWSER
void MayDrummerEditor::makeChips (std::vector<std::unique_ptr<juce::TextButton>>& v, const juce::StringArray& names,
                                  juce::String& target, int x, int y)
{
    for (int i = 0; i <= names.size(); ++i)
    {
        const juce::String value = i == 0 ? juce::String() : names[i - 1];
        auto b = std::make_unique<juce::TextButton> (i == 0 ? juce::String ("Tutti") : value);
        const int w = juce::jmax (52, uiFont (12.0f, true).getStringWidth (b->getButtonText()) + 22);
        b->setBounds (x, y, w, 22);
        x += w + 5;
        b->onClick = [this, &target, value] { target = value; rebuildBrowser(); };
        browserPage.addAndMakeVisible (*b);
        v.push_back (std::move (b));
    }
}

void MayDrummerEditor::buildBrowserPage()
{
    auto& pg = browserPage;
    pg.addAndMakeVisible (genreViewport);
    genreViewport.setBounds (24, 40, 216, 772);
    genreViewport.setViewedComponent (&genreList, false);
    genreViewport.setScrollBarsShown (true, false);
    genreViewport.setScrollBarThickness (8);
    genreList.onSelect = [this] (int id, int sub)
    {
        if (id >= 0 && sub < 0 && id == browserGenre && browserSub < 0) { browserGenre = -1; }
        else { browserGenre = id; }
        browserSub = sub;
        rebuildBrowser();
    };

    pg.addAndMakeVisible (searchBox);
    searchBox.setBounds (250, 40, 420, 32);
    searchBox.setTextToShowWhenEmpty ("Cerca: piu parole insieme (es. funk ghost intenso, bossa ride)", Colours::textDim);
    searchBox.setColour (juce::TextEditor::backgroundColourId, Colours::lcd);
    searchBox.setColour (juce::TextEditor::textColourId, Colours::cream);
    searchBox.setColour (juce::TextEditor::outlineColourId, Colours::panelLine);
    searchBox.setColour (juce::TextEditor::focusedOutlineColourId, Colours::gold);
    searchBox.setFont (uiFont (14.0f));
    searchBox.setJustification (juce::Justification::centredLeft);
    searchBox.setIndents (10, 0);
    searchBox.onTextChange = [this] { rebuildBrowser(); };

    pg.addAndMakeVisible (favOnly);
    favOnly.setBounds (690, 44, 160, 24);
    favOnly.onClick = [this] { rebuildBrowser(); };

    pg.addAndMakeVisible (browserCount);
    browserCount.setBounds (950, 44, 180, 24);
    browserCount.setJustificationType (juce::Justification::centredRight);
    browserCount.setColour (juce::Label::textColourId, Colours::textDim);

    caption ("CARATTERE", pg).setBounds (246, 84, 90, 18);
    makeChips (energyChips, { "Leggero", "Medio", "Intenso" }, fEnergy, 336, 82);
    caption ("FEEL", pg).setBounds (636, 84, 50, 18);
    makeChips (feelChips, { "Dritto", "Shuffle", "Half-time" }, fFeel, 686, 82);
    caption ("VARIANTE", pg).setBounds (246, 112, 90, 18);
    makeChips (variantChips, { "Base", "Colore", "Plus", "Sync", "Light", "Ghost", "Percussioni", "Push" }, fVariant, 336, 110);

    const char* heads[] = { "GENERE", "SOTTOGENERE", "PRESET", "ASCOLTA" };
    const int hx[] = { 290, 440, 660, 1072 };
    for (int i = 0; i < 4; ++i) caption (heads[i], pg).setBounds (hx[i], 138, 160, 18);

    pg.addAndMakeVisible (browserList);
    browserList.setBounds (250, 158, 882, 406);
    browserList.setRowHeight (32);
    browserList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xe60b0a09));
    browserList.setColour (juce::ListBox::outlineColourId, Colours::panelLine);
    browserList.setOutlineThickness (1);

    caption ("ANTEPRIMA DEL PRESET SELEZIONATO (MAIN)", pg).setBounds (246, 572, 500, 18);
    pg.addAndMakeVisible (browserView);
    browserView.setBounds (250, 596, 882, 168);
    browserView.source = [this] () -> const DrumSequence& { return proc.partSeqs[PMain]; };
    browserView.bpm = [this] { return proc.effectiveBpm(); };
    browserView.noteOf = [this] (int p) { return proc.getNote (p); };
    browserView.playhead = [this] { return (proc.getParam ("playMode") < 0.5f && (int) proc.getParam ("part") == PMain) ? proc.playPosition.load() : -1.0; };
    browserView.makeFiles = [this] { return juce::StringArray { proc.writePartFile (PMain).getFullPathName() }; };

    pg.addAndMakeVisible (browserPlay);
    pg.addAndMakeVisible (browserGo);
    browserPlay.setBounds (250, 774, 150, 34);
    browserGo.setBounds (410, 774, 210, 34);
    browserPlay.setClickingTogglesState (true);
    toggleAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "preview", browserPlay));
    browserPlay.onClick = [this] { proc.setParam ("part", 0.0f); proc.setParam ("playMode", 0.0f); };
    browserGo.onClick = [this] { showPage (0); };

    auto& h = caption ("Stellina = preferito. Doppio clic per aprirlo nel Drummer.", pg);
    h.setBounds (636, 776, 500, 32);
    h.setFont (uiFont (12.0f));
    h.setColour (juce::Label::textColourId, Colours::textDim);
    h.setMinimumHorizontalScale (1.0f);
}

void MayDrummerEditor::rebuildBrowser()
{
    auto& lib = PatternLibrary::get();
    juce::StringArray terms;
    terms.addTokens (searchBox.getText().toLowerCase(), " ", "");
    terms.removeEmptyStrings();
    const bool onlyFav = favOnly.getToggleState();

    auto& items = genreList.items;
    items.clear();
    items.push_back ({ -1, -1, "Tutti", lib.totalPatterns(), false });
    items.push_back ({ -2, -1, "Preferiti", proc.numFavourites(), false });
    items.push_back ({ -3, -1, "GENERI", 0, true });
    for (int g = 0; g < NumStyles; ++g)
    {
        items.push_back ({ g, -1, juce::String (g == browserGenre ? "- " : "+ ") + PatternLibrary::styleName (g),
                           PatternLibrary::subsPerStyle * PatternLibrary::presetsPerSub, false });
        if (g == browserGenre)
        {
            auto subs = lib.subNames (g);
            for (int s = 0; s < subs.size(); ++s) items.push_back ({ g, s, subs[s], PatternLibrary::presetsPerSub, false });
        }
    }
    genreList.selected = browserGenre;
    genreList.selectedSub = browserSub;
    genreList.setSize (200, (int) items.size() * 30 + 4);
    genreList.repaint();

    auto mark = [] (std::vector<std::unique_ptr<juce::TextButton>>& v, const juce::String& cur)
    { for (auto& b : v) b->setToggleState ((cur.isEmpty() && b->getButtonText() == "Tutti") || b->getButtonText() == cur, juce::dontSendNotification); };
    mark (energyChips, fEnergy);
    mark (feelChips, fFeel);
    mark (variantChips, fVariant);

    auto& rows = browserModel.rows;
    rows.clear();
    int count = 0;
    for (int g = 0; g < NumStyles; ++g)
        for (int s = 0; s < PatternLibrary::subsPerStyle; ++s)
        {
            std::vector<int> found;
            for (int p = 0; p < PatternLibrary::presetsPerSub; ++p)
            {
                const auto& pat = lib.getPattern (g, s, p);
                const bool fav = proc.isFavourite (g, s, p);
                if (browserGenre >= 0 && g != browserGenre) continue;
                if (browserSub >= 0 && s != browserSub) continue;
                if ((browserGenre == -2 || onlyFav) && ! fav) continue;
                if (fEnergy.isNotEmpty() && pat.energy != fEnergy) continue;
                if (fFeel.isNotEmpty() && pat.feel != fFeel) continue;
                if (fVariant.isNotEmpty() && pat.vgroup != fVariant) continue;
                bool ok = true;
                for (auto& t : terms) if (! pat.searchText.contains (t)) { ok = false; break; }
                if (ok) found.push_back (p);
            }
            if (found.empty()) continue;
            const auto& first = lib.getPattern (g, s, found[0]);
            rows.push_back ({ true, g, s, -1, juce::String (PatternLibrary::styleName (g)) + " / " + lib.subNames (g)[s]
                                              + "      " + juce::String ((int) found.size()) + " preset, " + first.feel });
            for (int p : found) rows.push_back ({ false, g, s, p, {} });
            count += (int) found.size();
        }
    browserCount.setText (juce::String (count) + " preset", juce::dontSendNotification);
    browserList.updateContent();
    browserList.repaint();
}

void MayDrummerEditor::BrowserModel::paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool)
{
    if (row < 0 || row >= (int) rows.size()) return;
    const auto& r = rows[(size_t) row];
    if (r.header)
    {
        g.fillAll (juce::Colour (0xff221d17));
        g.setColour (Colours::gold);
        g.setFont (uiFont (13.0f, true));
        g.drawText (r.text, 12, 0, w - 24, h, juce::Justification::centredLeft);
        return;
    }
    auto& proc = ed.proc;
    const bool current = r.g == (int) proc.getParam ("genre") && r.s == (int) proc.getParam ("sub") && r.p == (int) proc.getParam ("preset");
    if (current) g.fillAll (juce::Colour (0xff3a2d18));
    g.setColour (juce::Colour (0x0dffffff));
    g.drawHorizontalLine (h - 1, 0.0f, (float) w);

    drawStar (g, juce::Rectangle<float> (10.0f, (float) h * 0.5f - 9.0f, 18.0f, 18.0f), proc.isFavourite (r.g, r.s, r.p));
    const auto& pat = PatternLibrary::get().getPattern (r.g, r.s, r.p);
    g.setColour (current ? juce::Colours::white : Colours::cream);
    g.setFont (uiFont (14.0f, current));
    g.drawText (PatternLibrary::styleName (r.g), 40, 0, 150, h, juce::Justification::centredLeft);
    g.drawText (PatternLibrary::get().subNames (r.g)[r.s], 190, 0, 220, h, juce::Justification::centredLeft);
    g.drawText (pat.name, 410, 0, 180, h, juce::Justification::centredLeft);
    g.setColour (Colours::textDim);
    g.setFont (uiFont (12.0f));
    g.drawText (pat.energy, 590, 0, 120, h, juce::Justification::centredLeft);

    auto c = juce::Rectangle<float> ((float) w - 44.0f, (float) h * 0.5f - 12.0f, 24.0f, 24.0f);
    g.setColour (juce::Colour (0xff5a544b));
    g.drawEllipse (c, 1.5f);
    juce::Path tri;
    const auto cc = c.getCentre();
    tri.addTriangle (cc.x - 4.0f, cc.y - 5.5f, cc.x - 4.0f, cc.y + 5.5f, cc.x + 6.0f, cc.y);
    g.setColour (Colours::cream);
    g.fillPath (tri);
}

void MayDrummerEditor::BrowserModel::listBoxItemClicked (int row, const juce::MouseEvent& e)
{
    if (row < 0 || row >= (int) rows.size() || rows[(size_t) row].header) return;
    const auto& r = rows[(size_t) row];
    if (e.x < 38) { ed.proc.toggleFavourite (r.g, r.s, r.p); ed.rebuildBrowser(); return; }
    ed.proc.loadPattern (r.g, r.s, r.p);
    if (e.x > ed.browserList.getWidth() - 70)
    {
        ed.proc.setParam ("part", 0.0f);
        ed.proc.setParam ("playMode", 0.0f);
        ed.proc.setParam ("preview", 1.0f);
    }
    ed.browserList.repaint();
}

void MayDrummerEditor::BrowserModel::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    if (row < 0 || row >= (int) rows.size() || rows[(size_t) row].header) return;
    const auto& r = rows[(size_t) row];
    ed.proc.loadPattern (r.g, r.s, r.p);
    ed.showPage (0);
}

// ---------------------------------------------------------------- Impostazioni salvate
void MayDrummerEditor::askSaveName()
{
    // La finestra appartiene all'editor: se chiudi il plug-in, si chiude con lui (nessun crash)
    saveDialog = std::make_unique<juce::AlertWindow> ("Salva impostazioni",
                                                      "Dai un nome alle impostazioni attuali: genere, preset, misura, humanize, "
                                                      "May Drummer, mappa note e struttura della song.",
                                                      juce::MessageBoxIconType::NoIcon, nullptr);
    saveDialog->setLookAndFeel (&lnf);
    const auto current = proc.getUserPresetName();
    auto suggestion = current.isNotEmpty() ? current : proc.currentPattern().fullName.replace (" / ", " ");
    saveDialog->addTextEditor ("name", suggestion, "Nome");
    saveDialog->addButton ("SALVA", 1, juce::KeyPress (juce::KeyPress::returnKey));
    saveDialog->addButton ("ANNULLA", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    content.addAndMakeVisible (*saveDialog);
    saveDialog->setCentrePosition (designW / 2, designH / 2);

    juce::Component::SafePointer<MayDrummerEditor> safe (this);
    saveDialog->enterModalState (true, juce::ModalCallbackFunction::create ([safe] (int result)
    {
        if (safe == nullptr || safe->saveDialog == nullptr) return;
        auto& ed = *safe;
        const auto name = ed.saveDialog->getTextEditorContents ("name").trim();
        juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->saveDialog.reset(); });
        if (result != 1 || name.isEmpty()) return;
        if (ed.proc.listUserPresets().contains (name) && name != ed.proc.getUserPresetName())
        {
            juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                              .withTitle ("Sostituire?")
                                              .withMessage ("Esiste gia un'impostazione chiamata \"" + name + "\". Vuoi sostituirla?")
                                              .withButton ("SOSTITUISCI").withButton ("ANNULLA")
                                              .withAssociatedComponent (&ed),
                                          [safe, name] (int r)
                                          {
                                              if (safe == nullptr) return;
                                              if (r == 1) safe->proc.saveUserPreset (name);
                                              safe->content.repaint();
                                          });
            return;
        }
        ed.proc.saveUserPreset (name);
        ed.content.repaint();
    }), false);
}

void MayDrummerEditor::showLoadMenu()
{
    const auto names = proc.listUserPresets();
    const auto current = proc.getUserPresetName();
    juce::PopupMenu m, del;
    m.addSectionHeader ("IMPOSTAZIONI SALVATE");
    if (names.isEmpty())
        m.addItem (-1, "Nessuna impostazione salvata: usa SALVA", false);
    for (int i = 0; i < names.size(); ++i)
    {
        m.addItem (1 + i, names[i], true, names[i] == current);
        del.addItem (1001 + i, names[i]);
    }
    m.addSeparator();
    m.addSubMenu ("Elimina", del, ! names.isEmpty());
    m.addItem (2001, "Apri la cartella delle impostazioni");

    juce::Component::SafePointer<MayDrummerEditor> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&loadBtn), [safe, names] (int r)
    {
        if (safe == nullptr || r == 0) return;
        auto& ed = *safe;
        if (r >= 1 && r <= names.size())
        {
            ed.proc.loadUserPreset (names[r - 1]);
            ed.snapshotPreviews();
            ed.content.repaint();
        }
        else if (r >= 1001 && r <= 1000 + names.size())
        {
            const auto name = names[r - 1001];
            juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                              .withTitle ("Eliminare?")
                                              .withMessage ("Vuoi eliminare l'impostazione \"" + name + "\"?")
                                              .withButton ("ELIMINA").withButton ("ANNULLA")
                                              .withAssociatedComponent (&ed),
                                          [safe, name] (int res)
                                          {
                                              if (safe == nullptr) return;
                                              if (res == 1) safe->proc.deleteUserPreset (name);
                                              safe->content.repaint();
                                          });
        }
        else if (r == 2001)
        {
            auto dir = ed.proc.userPresetFolder();
            dir.createDirectory();
            dir.revealToUser();
        }
    });
}

void MayDrummerEditor::layoutDrummer() {}
void MayDrummerEditor::layoutSong() {}
void MayDrummerEditor::layoutMap() {}

namespace { const PatternLibrary& lib() { return PatternLibrary::get(); } }

void MayDrummerEditor::timerCallback()
{
    // tab in alto a destra
    tabDrummer.setBounds (designW - 396, 40, 90, 34);
    tabBrowser.setBounds (designW - 300, 40, 90, 34);
    tabSong.setBounds (designW - 204, 40, 90, 34);
    tabMap.setBounds (designW - 108, 40, 90, 34);

    const int gen = proc.generation.load();
    if (gen != lastGeneration)
    {
        lastGeneration = gen;
        refreshSubCombo();
        presets.repaint();
        mainView.repaint();
        songView.repaint();
        pad.repaint();
        content.repaint (0, 0, designW, 116);
        if (! pad.isMouseButtonDown()) snapshotPreviews();
        for (int i = 0; i < NumPieces; ++i)
            noteSliders[(size_t) i].setValue (proc.getNote (i), juce::dontSendNotification);
        if (page == 3) { browserList.repaint(); genreList.repaint(); browserView.repaint(); }
        {
            const auto& cp = proc.currentPattern();
            presetInfo.setText (juce::String::fromUTF8 ("\xc2\xb7  ") + lib().subNames (cp.style)[cp.sub] + ": " + cp.energy + ", " + cp.feel, juce::dontSendNotification);
        }
    }

    static const char* lvNames[] = { "ESSENZIALE", "MINIMO", "SEMPLICE", "PULITO", "ASCIUTTO", "ORIGINALE",
                                     "VIVO", "RICCO", "GROOVE", "ELABORATO", "VIRTUOSO" };
    const int lv = (int) proc.getParam ("level");
    lvLabel.setText ((lv > 0 ? "+" : "") + juce::String (lv) + "\n" + lvNames[lv + 5], juce::dontSendNotification);
    lvUp.setEnabled (lv < 5);
    lvDn.setEnabled (lv > -5);
    const bool alt = proc.getParam ("alt") > 0.5f;
    originalBtn.setEnabled (alt);
    shuffleBtn.setToggleState (alt, juce::dontSendNotification);
    for (auto& k : knobs)
    {
        const float v = proc.getParam (k->id);
        juce::String t = juce::String (juce::roundToInt (v)) + "%";
        if (k->id == "swing") t = v <= 50.5f ? "Dritto" : t;
        if (k->id == "feel") t = std::abs (v) < 0.5f ? "Centro" : (v < 0 ? "Push " : "Laid ") + juce::String (juce::roundToInt (std::abs (v)));
        k->value.setText (t, juce::dontSendNotification);
    }

    {
        const int pr = (int) proc.getParam ("preset");
        if (pr != lastPresetSeen) { uiPage = pr / PresetStrip::perPage; lastPresetSeen = pr; presets.repaint(); }
        pageLabel.setText (juce::String (uiPage * PresetStrip::perPage + 1) + "-" + juce::String (uiPage * PresetStrip::perPage + PresetStrip::perPage),
                           juce::dontSendNotification);
    }
    const int part = (int) proc.getParam ("part");
    {
        juce::String t;
        const auto st = proc.makeSettings();
        const auto& cp = proc.currentPattern();
        if (part >= PFill1 && part <= PFill3) t = juce::String (partName (part)) + ": " + DrumGenerator::fillName (DrumGenerator::fillRecipeFor (cp, st, part - PFill1));
        else if (part >= PEnd1) t = juce::String (partName (part)) + ": " + DrumGenerator::endingName (DrumGenerator::endingRecipeFor (cp, st, part - PEnd1));
        if (t != mainView.title) { mainView.title = t; mainView.repaint(); }
    }
    const bool loop = proc.getParam ("playMode") < 0.5f;
    for (int i = 0; i < NumParts; ++i)
        partBtns[(size_t) i].setToggleState (loop && i == part, juce::dontSendNotification);

    if (auto* m = byId["meter"]) m->setEnabled (proc.getParam ("meterHost") < 0.5f || ! proc.hostHasMeter());
    bpmSlider.setEnabled (proc.getParam ("bpmHost") < 0.5f || ! proc.hostHasBpm());

    if (proc.playPosition.load() >= 0.0)
    {
        if (page == 0) mainView.repaint();
        if (page == 3) browserView.repaint();
        if (page == 1) songView.repaint();
    }
}
