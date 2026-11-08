#pragma once
#include <juce_core/juce_core.h>
#include <array>
#include <vector>

namespace may
{
// Strumenti del kit virtuale
enum Piece
{
    Kick = 0, Snare, SideStick, HHClosed, HHOpen, HHPedal, Ride, RideBell, Crash,
    TomHi, TomMid, TomFloor, Clap, Tambourine, Cowbell, Shaker, China, NumPieces
};

inline const char* pieceName (int p)
{
    static const char* n[] = { "Cassa", "Rullante", "Sidestick", "HH Chiuso", "HH Aperto", "HH Pedale",
                               "Ride", "Campana Ride", "Crash", "Tom Alto", "Tom Medio", "Timpano",
                               "Clap", "Tamburello", "Campanaccio", "Shaker", "China" };
    return n[p];
}

// Mappa General MIDI (compatibile con Addictive Drums, EZdrummer, Superior, BFD in modalita GM)
inline int gmNote (int p)
{
    static const int n[] = { 36, 38, 37, 42, 46, 44, 51, 53, 49, 50, 47, 43, 39, 54, 56, 70, 52 };
    return n[p];
}

inline int pieceFromChar (char c)
{
    switch (c)
    {
        case 'K': return Kick;      case 'S': return Snare;     case 'X': return SideStick;
        case 'H': return HHClosed;  case 'O': return HHOpen;    case 'P': return HHPedal;
        case 'R': return Ride;      case 'B': return RideBell;  case 'C': return Crash;
        case '1': return TomHi;     case '2': return TomMid;    case '3': return TomFloor;
        case 'L': return Clap;      case 'A': return Tambourine;case 'Q': return Cowbell;
        case 'M': return Shaker;    case 'Y': return China;
        default:  return -1;
    }
}

inline int velFromChar (char c)
{
    switch (c)
    {
        case 'X': return 120; case 'x': return 100; case 'o': return 78; case 'g': return 42;
        default: return 0;
    }
}

// Gruppi per il mute del kit
enum KitGroup { GKick = 0, GSnare, GHats, GToms, GCymbals, GPerc, NumGroups };
inline const char* groupName (int g)
{
    static const char* n[] = { "Cassa", "Rullante", "Hi-Hat", "Tom", "Piatti", "Percussioni" };
    return n[g];
}

inline int groupOf (int p)
{
    switch (p)
    {
        case Kick: return GKick;
        case Snare: case SideStick: return GSnare;
        case HHClosed: case HHOpen: case HHPedal: return GHats;
        case TomHi: case TomMid: case TomFloor: return GToms;
        case Ride: case RideBell: case Crash: case China: return GCymbals;
        default: return GPerc;
    }
}

enum Style
{
    SRock = 0, SPop, SFunk, SBlues, SCountry, SJazz, SMetal, SPunk, SReggae, SHipHop,
    SSoul, SDisco, SLatin, SBallad, SMotown, SElectronic, NumStyles
};

enum PartType { PMain = 0, PRiff, PFill1, PFill2, PFill3, PEnd1, PEnd2, PEnd3, NumParts };

inline const char* partName (int p)
{
    static const char* n[] = { "Main", "Riff", "Fill 1", "Fill 2", "Fill 3", "Finale 1", "Finale 2", "Finale 3" };
    return n[p];
}

struct DrumEvent
{
    double ppq = 0;     // posizione in quarti
    int piece = 0;
    int vel = 100;
    float ms = 0;       // spostamento umanizzato in millisecondi
};

struct DrumSequence
{
    std::vector<DrumEvent> events;
    double lengthPPQ = 8.0;
    double barPPQ = 4.0;
    int numBars = 2;
    std::vector<int> barKinds;   // per la vista Song: 0 main, 1 riff, 2 fill, 3 finale, 4 count-in
    int meterNum = 4, meterDen = 4;
};

struct GenSettings
{
    int meterNum = 4, meterDen = 4;
    int bars = 2;
    int feelMode = 0;            // 0 auto (come il pattern), 1 dritto, 2 shuffle
    float swing = 0.62f;
    bool swing16 = false;
    int timeFeel = 0;            // 0 normale, 1 half-time, 2 double-time
    float humTiming = 0.25f, humVelocity = 0.3f, volume = 1.0f, intensity = 0.6f;
    float feel = 0.0f;           // -1 push ... +1 laid back
    float ghost = 0.5f, complexity = 0.3f, hatAccent = 0.5f;
    int cymbalMode = 0;          // 0 auto, 1 hi-hat, 2 ride
    int level = 0;               // -5 semplifica ... +5 arricchisci
    int sideStick = 0;           // 0 normale, 1 side stick nel Main, 2 sempre
    bool crashOnOne = true;
    juce::uint32 seed = 1;
    std::array<bool, NumGroups> mute {};
    // Song
    int songBars = 32, sectionBars = 8, fillEvery = 4, ending = 1;
    bool countIn = false, altRiff = true;
};
} // namespace may
