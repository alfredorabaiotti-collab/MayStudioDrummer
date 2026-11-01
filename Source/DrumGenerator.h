#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "PatternLibrary.h"

namespace may
{
class DrumGenerator
{
public:
    // Una parte (Main, Riff, Fill 1-3, Finale 1-3) ripetibile in loop
    static DrumSequence renderPart (const Pattern& p, int part, const GenSettings& s);

    // Una traccia completa: count-in, strofe/ritornelli, fill, finale
    static DrumSequence renderSong (const Pattern& p, const GenSettings& s);

    static juce::MidiFile toMidiFile (const DrumSequence& seq, const std::array<int, NumPieces>& noteMap,
                                      int channel, double bpm, int num, int den, const juce::String& name);

    struct Bar
    {
        int steps = 16, grid = 4, stepsPerBeat = 4;
        double stepPPQ = 0.25;
        std::array<std::vector<int>, NumPieces> v;
        void init (int n) { steps = n; for (auto& l : v) l.assign ((size_t) n, 0); }
        int& at (int p, int s) { return v[(size_t) p][(size_t) s]; }
        int get (int p, int s) const { return v[(size_t) p][(size_t) s]; }
        bool uses (int p) const { for (auto x : v[(size_t) p]) if (x > 0) return true; return false; }
        void clearRange (int from, int to, bool keepPedal = true)
        {
            for (int p = 0; p < NumPieces; ++p)
                if (! (keepPedal && p == HHPedal))
                    for (int i = juce::jmax (0, from); i < juce::jmin (to, steps); ++i) v[(size_t) p][(size_t) i] = 0;
        }
    };

    enum Special { None = 0, StopBar, RollBar, HitBar1, HitBar2, HitBar3, CountIn };

private:
    static Bar emptyBar (const Pattern& p, const GenSettings& s);
    static Bar groove (const Pattern& p, const GenSettings& s, int barIdx, juce::Random& r);
    static void adaptMeter (const Pattern& p, const GenSettings& s, int barIdx, Bar& b);
    static void applyRiff (Bar& b, const Pattern& p, const GenSettings& s, juce::Random& r);
    static void applyFill (Bar& b, const Pattern& p, const GenSettings& s, int fillType, juce::Random& r);
    static void applyLevel (Bar& b, const Pattern& p, const GenSettings& s, juce::Random& r);
    static void applySpecial (Bar& b, const Pattern& p, const GenSettings& s, int special, juce::Random& r);
    static Bar makeBar (const Pattern& p, const GenSettings& s, int barIdx, bool riff, int fill, int special,
                        bool crash, juce::Random& r);
    static void emit (const Bar& b, double startPPQ, double scale, const GenSettings& s, juce::Random& r,
                      std::vector<DrumEvent>& out);
    static void emitBar (const Pattern& p, const GenSettings& s, int barIdx, bool riff, int fill, int special,
                         bool crash, double startPPQ, juce::Random& r, std::vector<DrumEvent>& out);
    static double barLengthPPQ (const GenSettings& s);
    static float effectiveSwing (const Pattern& p, const GenSettings& s, bool& shuffle);
};
} // namespace may
