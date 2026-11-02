#pragma once
#include "PluginProcessor.h"
#include "MayLookAndFeel.h"

namespace may
{
// Qualsiasi componente da cui si puo trascinare MIDI nella DAW
class MidiDragSource : public juce::Component, public juce::SettableTooltipClient
{
public:
    std::function<juce::StringArray()> makeFiles;
    void mouseDown (const juce::MouseEvent&) override { dragging = false; }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (! dragging && makeFiles && e.getDistanceFromDragStart() > 5)
        {
            dragging = true;
            auto files = makeFiles();
            if (! files.isEmpty())
                juce::DragAndDropContainer::performExternalDragDropOfFiles (files, false, this);
        }
    }
    void mouseUp (const juce::MouseEvent&) override { dragging = false; }
    void mouseEnter (const juce::MouseEvent&) override { if (makeFiles) setMouseCursor (juce::MouseCursor::DraggingHandCursor); }
protected:
    bool dragging = false;
};

// Grande pulsante dorato "Trascina nella DAW"
class DragZone : public MidiDragSource
{
public:
    DragZone (juce::String t, juce::String sub) : title (std::move (t)), subtitle (std::move (sub)) {}
    void paint (juce::Graphics& g) override;
    void mouseEnter (const juce::MouseEvent& e) override { hover = true; repaint(); MidiDragSource::mouseEnter (e); }
    void mouseExit (const juce::MouseEvent&) override { hover = false; repaint(); }
private:
    juce::String title, subtitle;
    bool hover = false;
};

// Visualizzazione di una sequenza (stile piano-roll di batteria)
class PatternView : public MidiDragSource
{
public:
    std::function<const DrumSequence&()> source;
    std::function<double()> playhead;
    std::function<double()> bpm;   // per disegnare gli spostamenti di timing umanizzati
    std::function<int (int)> noteOf;   // nota MIDI di ogni strumento, mostrata accanto al nome
    juce::String title;
    bool compact = false;
    juce::Colour accent = Colours::gold;
    void paint (juce::Graphics& g) override;
};

// Le 5 tessere dei preset del sottogenere
class PresetStrip : public juce::Component
{
public:
    std::function<juce::StringArray()> names;
    std::function<int()> selected;
    std::function<void (int)> onSelect;
    std::function<bool (int)> isFav;
    std::function<int()> firstNumber;   // numero del primo preset della pagina
    static constexpr int perPage = 10;
    std::function<void (int)> onStar;
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override { hoverIdx = indexAt (e.x); repaint(); }
    void mouseExit (const juce::MouseEvent&) override { hoverIdx = -1; repaint(); }
private:
    int indexAt (int x) const;
    int hoverIdx = -1;
};

// May Drummer: il quadrato con il puntino
class XYPad : public juce::Component
{
public:
    explicit XYPad (MayDrummerProcessor& p) : proc (p) {}
    std::function<void()> onRelease;
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
private:
    juce::Rectangle<float> padArea() const { return getLocalBounds().toFloat().reduced (24.0f, 20.0f); }
    void setFrom (juce::Point<float> pt);
    MayDrummerProcessor& proc;
    bool active = false;
};

struct Knob
{
    juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox };
    juce::Label label, value;
    juce::String id;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
};

void drawStar (juce::Graphics& g, juce::Rectangle<float> r, bool on);

// Pulsante con triangolo disegnato (nessun font speciale richiesto)
class ArrowBtn : public juce::Button
{
public:
    explicit ArrowBtn (bool upwards) : juce::Button (upwards ? "su" : "giu"), up (upwards) {}
    void paintButton (juce::Graphics& g, bool over, bool down) override
    {
        getLookAndFeel().drawButtonBackground (g, *this, {}, over, down);
        auto c = getLocalBounds().toFloat().getCentre();
        juce::Path t;
        const float s = 8.0f;
        if (up) t.addTriangle (c.x - s, c.y + s * 0.6f, c.x + s, c.y + s * 0.6f, c.x, c.y - s * 0.7f);
        else    t.addTriangle (c.x - s, c.y - s * 0.6f, c.x + s, c.y - s * 0.6f, c.x, c.y + s * 0.7f);
        g.setColour (isEnabled() ? Colours::gold : Colours::textDim.withAlpha (0.4f));
        g.fillPath (t);
    }
private:
    bool up;
};

// Elenco generi del browser
class GenreList : public juce::Component
{
public:
    struct Item { int id; int sub; juce::String name; int count; bool header; };
    std::vector<Item> items;
    int selected = -1, selectedSub = -1;
    std::function<void (int, int)> onSelect;
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
};
} // namespace may

class MayDrummerEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MayDrummerEditor (MayDrummerProcessor&);
    ~MayDrummerEditor() override;
    void paint (juce::Graphics&) override {}
    void resized() override;

private:
    static constexpr int designW = 1180, designH = 966;

    class Content : public juce::Component
    {
    public:
        explicit Content (MayDrummerEditor& e) : ed (e) {}
        void paint (juce::Graphics& g) override;
        MayDrummerEditor& ed;
    };

    void timerCallback() override;
    void buildDrummerPage();
    void buildSongPage();
    void buildMapPage();
    void buildBrowserPage();
    void rebuildBrowser();
    void layoutDrummer();
    void layoutSong();
    void layoutMap();
    void showPage (int p);
    void refreshSubCombo();
    void snapshotPreviews();
    void paintSection (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& number, const juce::String& title);
    void paintStudio (juce::Graphics& g, juce::Rectangle<int> r);

    juce::ComboBox& combo (const juce::String& id, juce::Component& parent);
    juce::ToggleButton& toggle (const juce::String& id, const juce::String& text, juce::Component& parent);
    may::Knob& knob (const juce::String& id, const juce::String& text, juce::Component& parent);
    juce::Label& caption (const juce::String& text, juce::Component& parent);

    MayDrummerProcessor& proc;
    may::MayLookAndFeel lnf;
    Content content { *this };
    juce::Image logo;
    juce::TooltipWindow tooltips { this, 600 };
    std::unique_ptr<juce::AlertWindow> saveDialog;

    juce::TextButton saveBtn { "SALVA" }, loadBtn { "RICHIAMA" };
    void askSaveName();
    void showLoadMenu();
    juce::TextButton tabDrummer { "DRUMMER" }, tabBrowser { "BROWSER" }, tabSong { "SONG" }, tabMap { "MAPPA" };
    int page = 0;
    juce::Component drummerPage, songPage, mapPage, browserPage;

    // Browser
    class BrowserModel : public juce::ListBoxModel
    {
    public:
        explicit BrowserModel (MayDrummerEditor& e) : ed (e) {}
        struct Row { bool header; int g, s, p; juce::String text; };
        std::vector<Row> rows;
        int getNumRows() override { return (int) rows.size(); }
        void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected) override;
        void listBoxItemClicked (int row, const juce::MouseEvent& e) override;
        void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;
        MayDrummerEditor& ed;
    };
    BrowserModel browserModel { *this };
    juce::ListBox browserList { "preset", &browserModel };
    may::GenreList genreList;
    juce::Viewport genreViewport;
    std::vector<std::unique_ptr<juce::TextButton>> energyChips, feelChips, variantChips;
    juce::String fEnergy, fFeel, fVariant;
    int browserSub = -1;
    void makeChips (std::vector<std::unique_ptr<juce::TextButton>>& v, const juce::StringArray& names, juce::String& target, int x, int y);
    juce::TextEditor searchBox;
    juce::ToggleButton favOnly { "Solo preferiti" };
    may::PatternView browserView;
    juce::TextButton browserPlay { "ASCOLTA" }, browserGo { "USA NEL DRUMMER" };
    juce::Label browserCount;
    int browserGenre = -1;

    may::ArrowBtn lvUp { true }, lvDn { false };
    juce::Label lvLabel;
    std::vector<std::unique_ptr<may::DragZone>> instrumentZones;
    void addInstrumentRow (juce::Component& parent, int x, int y, int w, bool song);
    juce::TextButton shuffleBtn { "RITMO ALTERNATIVO" }, originalBtn { "ORIGINALE" };

    // Pagina Drummer
    juce::ComboBox* genreBox = nullptr;
    juce::ComboBox subBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, diceBtn { "CASUALE" }, diceGenreBtn { "TUTTO CASUALE" };
    may::PresetStrip presets;
    std::array<juce::TextButton, may::NumParts> partBtns;
    may::PatternView mainView;
    may::DragZone dragPart { "TRASCINA LA PARTE", "nella traccia MIDI della DAW" };
    may::DragZone dragAll { "TUTTE LE PARTI", "Main, Riff, 3 Fill, 3 Finali" };
    juce::TextButton previewBtn { "ANTEPRIMA" };
    may::XYPad pad { proc };
    std::array<may::PatternView, 3> mdPreview;
    std::array<may::DrumSequence, 3> mdSeqs;
    juce::Label mdHint;
    juce::Slider kitVolSlider { juce::Slider::LinearBar, juce::Slider::TextBoxLeft };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kitVolAtt;
    juce::TextEditor quickSearch;
    juce::TextButton pagePrev { "<" }, pageNext { ">" };
    juce::Label pageLabel;
    int uiPage = 0, lastPresetSeen = -1;
    juce::Label presetInfo;
    juce::Slider bpmSlider { juce::Slider::LinearBar, juce::Slider::TextBoxLeft };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpmAtt;

    // Pagina Song
    may::PatternView songView;
    may::DragZone dragSong { "TRASCINA LA SONG", "traccia completa con fill e finale" };
    juce::Slider songBarsSlider { juce::Slider::LinearBar, juce::Slider::TextBoxLeft };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> songBarsAtt;

    // Pagina Mappa
    std::array<juce::Slider, may::NumPieces> noteSliders;
    std::array<juce::Label, may::NumPieces> noteLabels;
    juce::TextButton resetMapBtn { "RESET GENERAL MIDI" };
    juce::Slider channelSlider { juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> channelAtt;
    juce::Label mapHelp;

    std::vector<std::unique_ptr<juce::ComboBox>> combos;
    std::vector<std::unique_ptr<juce::ToggleButton>> toggles;
    std::vector<std::unique_ptr<may::Knob>> knobs;
    std::vector<std::unique_ptr<juce::Label>> captions;
    std::map<juce::String, juce::Component*> byId;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> toggleAtts;

    int lastGeneration = -1, lastGenre = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MayDrummerEditor)
};
