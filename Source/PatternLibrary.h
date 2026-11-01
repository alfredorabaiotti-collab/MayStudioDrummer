#pragma once
#include "DrumTypes.h"

namespace may
{
// Un pattern della libreria: una o due battute di riferimento in 4/4.
struct Pattern
{
    juce::String name;        // nome del preset
    juce::String fullName;    // Genere / Sottogenere / Preset
    int style = 0;
    int sub = 0;
    int grid = 4;             // suddivisioni per quarto: 4 = sedicesimi, 3 = terzine
    int stepsPerBar = 16;
    int coreBars = 1;
    juce::uint32 id = 0;      // seme stabile del pattern
    std::array<std::vector<juce::uint8>, NumPieces> lines;

    int at (int piece, int bar, int step) const
    {
        auto& l = lines[(size_t) piece];
        if (l.empty()) return 0;
        return l[(size_t) (((bar % coreBars) * stepsPerBar + step) % (int) l.size())];
    }
    bool uses (int piece) const
    {
        for (auto v : lines[(size_t) piece]) if (v > 0) return true;
        return false;
    }
};

// Libreria: 16 generi x 4 sottogeneri x 5 preset = 320 pattern
class PatternLibrary
{
public:
    static const PatternLibrary& get();

    static constexpr int subsPerStyle = 4;
    static constexpr int presetsPerSub = 5;

    static const char* styleName (int s);
    static juce::StringArray styleNames();
    juce::StringArray subNames (int style) const;
    juce::StringArray presetNames (int style, int sub) const;

    const Pattern& getPattern (int style, int sub, int preset) const;

    // "Ritmo alternativo": ricombina cassa, piatti e percussioni con altri groove dello stesso genere
    Pattern makeAlternative (const Pattern& p, juce::uint32 alt) const;
    int totalPatterns() const { return NumStyles * subsPerStyle * presetsPerSub; }

private:
    PatternLibrary();
    std::vector<std::vector<std::vector<Pattern>>> patterns;   // [genere][sottogenere][preset]
    std::vector<juce::StringArray> subs;
};
} // namespace may
