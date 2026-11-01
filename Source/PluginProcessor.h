#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "DrumGenerator.h"
#include "DrumSynth.h"
#include <set>

class MayDrummerProcessor : public juce::AudioProcessor,
                            private juce::AudioProcessorValueTreeState::Listener,
                            private juce::AsyncUpdater,
                            private juce::Timer
{
public:
    MayDrummerProcessor();
    ~MayDrummerProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override { return true; }
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "May Studio Midi Drummer"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ---- usati dall'interfaccia ----
    juce::AudioProcessorValueTreeState apvts;
    static juce::StringArray meterNames();

    const may::Pattern& currentPattern() const;
    may::GenSettings makeSettings() const;
    double effectiveBpm() const;
    void effectiveMeter (int& num, int& den) const;
    bool hostHasBpm() const { return hostBpm.load() > 1.0; }
    bool hostHasMeter() const { return hostNum.load() > 0; }

    // Sequenze per la grafica (solo thread dei messaggi)
    may::DrumSequence viewSeq, songSeq;
    std::array<may::DrumSequence, may::NumParts> partSeqs;
    std::atomic<int> generation { 0 };
    std::atomic<double> playPosition { -1.0 };   // 0..1 nella sequenza in riproduzione

    // File MIDI temporanei per il trascinamento nella DAW
    juce::File writePartFile (int part);
    juce::File writeSongFile();
    juce::StringArray writeAllPartFiles();
    // Singoli strumenti: group = 0..5, oppure -1 = un file per ogni strumento presente
    juce::StringArray writeInstrumentFiles (bool song, int group);

    int getNote (int piece) const { return noteMap[(size_t) piece].load(); }
    void setNote (int piece, int note);
    void resetNoteMap();

    void setParam (const juce::String& id, float plainValue);
    float getParam (const juce::String& id) const { return apvts.getRawParameterValue (id)->load(); }
    void randomise (bool alsoGenre);

    void regenerateNow();

    // Preferiti (salvati in un file, validi per tutti i progetti)
    static juce::String favKey (int g, int s, int p) { return juce::String (g) + ":" + juce::String (s) + ":" + juce::String (p); }
    bool isFavourite (int g, int s, int p) const { return favourites.count (favKey (g, s, p)) > 0; }
    void toggleFavourite (int g, int s, int p);
    int numFavourites() const { return (int) favourites.size(); }
    void loadPattern (int g, int s, int p);

    // Impostazioni salvate dall'utente, con nome (valide per tutti i progetti)
    juce::File userPresetFolder() const;
    juce::StringArray listUserPresets() const;
    bool saveUserPreset (const juce::String& name);
    bool loadUserPreset (const juce::String& name);
    bool deleteUserPreset (const juce::String& name);
    juce::String currentUserPreset;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void parameterChanged (const juce::String&, float) override;
    void handleAsyncUpdate() override;
    void timerCallback() override;
    juce::File makeTempFile (const juce::String& suffix) const;
    juce::String baseFileName() const;
    juce::File writeSequence (const may::DrumSequence& seq, const juce::String& suffix);

    std::array<std::atomic<int>, may::NumPieces> noteMap;
    std::set<juce::String> favourites;
    juce::File favouritesFile() const;
    void loadFavourites();
    mutable may::Pattern altPattern;
    mutable juce::String altKey;
    juce::String lastPatternKey;

    may::DrumSynth synth;
    juce::SpinLock seqLock;
    may::DrumSequence playSeq;    // thread audio

    std::atomic<double> hostBpm { 0.0 };
    std::atomic<int> hostNum { 0 }, hostDen { 0 };
    std::atomic<int> pendingPart { -1 };
    std::atomic<bool> mapChanged { false };
    mutable int exportCounter = 0;
    int lastHostNum = 0, lastHostDen = 0;
    double lastHostBpm = 0;

    double sampleRate = 44100.0;
    double internalPPQ = 0.0;
    bool wasPlaying = false;

    struct PendingOff { int note = 0, channel = 10; int samplesLeft = 0; };
    std::array<PendingOff, 512> pendingOffs;
    int numPendingOffs = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MayDrummerProcessor)
};
