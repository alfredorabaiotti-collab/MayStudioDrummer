#include "DrumGenerator.h"

namespace may
{
namespace
{
juce::int64 mixSeed (juce::uint32 a, juce::uint32 b, juce::uint32 c)
{
    juce::uint64 x = (juce::uint64) a * 0x9E3779B97F4A7C15ULL ^ ((juce::uint64) b << 21) ^ ((juce::uint64) c * 0xBF58476D1CE4E5B9ULL);
    x ^= x >> 31;
    return (juce::int64) (x & 0x7fffffffffffLL);
}

bool chance (juce::Random& r, float p) { return r.nextFloat() < p; }

void setMax (DrumGenerator::Bar& b, int piece, int step, int vel)
{
    if (step < 0 || step >= b.steps) return;
    auto& v = b.at (piece, step);
    v = juce::jmax (v, vel);
}
} // namespace

double DrumGenerator::barLengthPPQ (const GenSettings& s)
{
    return s.meterNum * 4.0 / (double) s.meterDen * (s.timeFeel == 1 ? 2.0 : 1.0);
}

float DrumGenerator::effectiveSwing (const Pattern& p, const GenSettings& s, bool& shuffle)
{
    const float knob = juce::jlimit (0.5f, 0.75f, s.swing);
    if (s.feelMode == 1) { shuffle = false; return 0.5f; }
    if (s.feelMode == 2) { shuffle = true;  return knob > 0.505f ? knob : 2.0f / 3.0f; }
    // Auto: i pattern in terzine sono shuffle; gli altri seguono il knob Swing (50% = dritto)
    if (p.grid == 3 && s.meterDen == 4) { shuffle = true; return knob > 0.505f ? knob : 2.0f / 3.0f; }
    shuffle = knob > 0.505f;
    return knob;
}

DrumGenerator::Bar DrumGenerator::emptyBar (const Pattern& p, const GenSettings& s)
{
    Bar b;
    if (s.meterDen == 8)
    {
        b.grid = 4; b.stepPPQ = 0.25; b.stepsPerBeat = 2;
        b.init (juce::jmax (2, s.meterNum * 2));
    }
    else
    {
        b.grid = p.grid; b.stepPPQ = 1.0 / p.grid; b.stepsPerBeat = p.grid;
        b.init (juce::jmax (1, s.meterNum) * p.grid);
    }
    return b;
}

// Adatta il groove di riferimento (4/4) a qualunque misura: 2/4, 3/4, 5/4, 6/8, 7/8, 9/8, 12/8...
void DrumGenerator::adaptMeter (const Pattern& p, const GenSettings& s, int barIdx, Bar& b)
{
    const int num = s.meterNum;
    const bool den8 = (s.meterDen == 8);

    auto coreVal = [&] (int piece, int beat, int k) -> int
    {
        if (! den8) return p.at (piece, barIdx, (beat % 4) * p.grid + k);
        if (p.grid == 4) return p.at (piece, barIdx, (beat * 2 + k) % 16);
        return k == 0 ? p.at (piece, barIdx, (beat % 4) * 3) : 0;
    };

    // 1) Piatti e percussioni ricopiati per ogni pulsazione
    const int timePieces[] = { HHClosed, HHOpen, Ride, RideBell, Shaker, Tambourine, Cowbell };
    for (int beat = 0; beat < num; ++beat)
        for (int k = 0; k < b.stepsPerBeat; ++k)
            for (int piece : timePieces)
                b.at (piece, beat * b.stepsPerBeat + k) = coreVal (piece, beat, k);

    // Crash usato come piatto di tempo (metal, punk)
    int crashCount = 0;
    for (int i = 0; i < p.stepsPerBar; ++i) if (p.at (Crash, 0, i) > 0) ++crashCount;
    if (crashCount >= 4)
        for (int beat = 0; beat < num; ++beat)
            b.at (Crash, beat * b.stepsPerBeat) = 100;

    // 2) Ossatura cassa / rullante
    const int snPiece = p.uses (Snare) ? (int) Snare : (p.uses (SideStick) ? (int) SideStick : (int) Clap);
    std::vector<std::pair<int, int>> backbone;   // step, strumento
    if (! den8)
    {
        const int g = b.stepsPerBeat;
        const bool waltz = (p.style == SJazz || p.style == SCountry || p.style == SLatin || p.style == SBlues || p.style == SBallad);
        if (num == 3)
        {
            backbone.push_back ({ 0, Kick });
            if (waltz) { backbone.push_back ({ g, -snPiece }); backbone.push_back ({ 2 * g, -snPiece }); }
            else backbone.push_back ({ 2 * g, snPiece });
        }
        else
        {
            for (int beat = 0; beat < num; ++beat)
                backbone.push_back ({ beat * g, (beat % 2 == 0) ? (int) Kick : snPiece });
        }
    }
    else
    {
        // raggruppamenti in crome: 3+3... oppure 2+2+...+3
        std::vector<int> groups;
        if (num % 3 == 0) for (int i = 0; i < num / 3; ++i) groups.push_back (3);
        else
        {
            int rem = num;
            while (rem > 3) { groups.push_back (2); rem -= 2; }
            groups.push_back (rem);
        }
        int pos = 0;
        for (size_t gi = 0; gi < groups.size(); ++gi)
        {
            backbone.push_back ({ pos * 2, (gi % 2 == 0) ? (int) Kick : snPiece });
            pos += groups[gi];
        }
    }

    for (auto& bb : backbone)
    {
        const bool soft = bb.second < 0;
        const int piece = std::abs (bb.second);
        b.at (piece, bb.first) = soft ? 78 : (piece == Kick ? 110 : 118);
        if (piece != Kick && p.uses (Clap) && piece != Clap) b.at (Clap, bb.first) = 100;
        if (piece != Kick && p.uses (HHPedal)) b.at (HHPedal, bb.first) = 80;
    }

    // 3) Sincopi di cassa e ghost notes del groove originale, dove cadono nella misura
    int kickCount = 0;
    for (int i = 0; i < p.stepsPerBar; ++i) if (p.at (Kick, 0, i) > 0) ++kickCount;
    if (kickCount >= 12)
    {
        for (int i = 0; i < b.steps; ++i) b.at (Kick, i) = 100;   // doppia cassa continua
    }
    else if (! den8)
    {
        for (int i = 0; i < p.stepsPerBar && i < b.steps; ++i)
        {
            if (i % p.grid != 0 && p.at (Kick, barIdx, i) > 0 && b.get (snPiece, i) == 0) b.at (Kick, i) = p.at (Kick, barIdx, i);
            if (p.at (Snare, barIdx, i) > 0 && p.at (Snare, barIdx, i) < 60) b.at (Snare, i) = p.at (Snare, barIdx, i);
        }
    }
}

DrumGenerator::Bar DrumGenerator::groove (const Pattern& p, const GenSettings& s, int barIdx, juce::Random& r)
{
    Bar b = emptyBar (p, s);
    const bool fourFour = (s.meterNum == 4 && s.meterDen == 4);

    if (fourFour)
    {
        for (int piece = 0; piece < NumPieces; ++piece)
            for (int i = 0; i < b.steps; ++i)
                b.at (piece, i) = p.at (piece, barIdx, i);
    }
    else
    {
        adaptMeter (p, s, barIdx, b);
    }

    const int L = b.steps;
    const int g = b.grid;

    // Seconda battuta: piccola variazione di cassa (stabile per ogni pattern)
    if (barIdx % 2 == 1 && g == 4 && p.style != SReggae && p.style != SElectronic && p.style != SMetal
        && ((p.id >> 1) & 1) && b.get (Kick, L - 2) == 0 && b.get (Snare, L - 2) == 0)
        b.at (Kick, L - 2) = 82;

    // Imprevedibilita: variazioni di cassa, hi-hat aperti, ghost notes, piccoli abbellimenti
    const float cx = s.complexity;
    if (cx > 0.01f)
    {
        for (int i = 0; i < L; ++i)
        {
            const bool offbeat = (i % b.stepsPerBeat) != 0;
            if (offbeat && (g == 3 || i % 2 == 0) && b.get (Kick, i) == 0 && b.get (Snare, i) < 90
                && p.style != SJazz && chance (r, cx * 0.12f))
                b.at (Kick, i) = 70 + r.nextInt (25);

            if (offbeat && b.get (HHClosed, i) > 0 && chance (r, cx * 0.07f))
            { b.at (HHOpen, i) = 92; b.at (HHClosed, i) = 0; }
        }
        if (barIdx % 2 == 1 && chance (r, cx * 0.35f) && g == 4 && L >= 4)
        {
            b.at (Snare, L - 1) = juce::jmax (b.get (Snare, L - 1), 70);
            if (chance (r, 0.5f)) b.at (Snare, L - 2) = juce::jmax (b.get (Snare, L - 2), 60);
        }
    }

    // Ghost notes
    if (s.ghost < 0.12f)
    {
        for (int i = 0; i < L; ++i) if (b.get (Snare, i) > 0 && b.get (Snare, i) < 55) b.at (Snare, i) = 0;
    }
    else if (s.ghost > 0.5f && p.style != SMetal && p.style != SElectronic && p.style != SPunk)
    {
        const float prob = (s.ghost - 0.5f) * 0.45f * (0.3f + cx);
        for (int i = 0; i < L; ++i)
        {
            if (b.get (Snare, i) > 0 || b.get (SideStick, i) > 0) continue;
            const bool nearBackbeat = (i > 0 && b.get (Snare, i - 1) >= 90) || (i + 1 < L && b.get (Snare, i + 1) >= 90);
            if (! nearBackbeat && (i % b.stepsPerBeat) != 0 && chance (r, prob))
                b.at (Snare, i) = 38 + r.nextInt (12);
        }
    }

    // Piatti forzati
    if (s.cymbalMode == 1)
        for (int i = 0; i < L; ++i)
        {
            if (b.get (Ride, i) > 0) { b.at (HHClosed, i) = juce::jmax (b.get (HHClosed, i), b.get (Ride, i)); b.at (Ride, i) = 0; }
            if (b.get (RideBell, i) > 0) { b.at (HHOpen, i) = b.get (RideBell, i); b.at (RideBell, i) = 0; }
        }
    else if (s.cymbalMode == 2)
        for (int i = 0; i < L; ++i)
        {
            if (b.get (HHClosed, i) > 0) { b.at (Ride, i) = juce::jmax (b.get (Ride, i), juce::jmax (b.get (HHClosed, i), 80)); b.at (HHClosed, i) = 0; }
            if (b.get (HHOpen, i) > 0) { b.at (RideBell, i) = b.get (HHOpen, i); b.at (HHOpen, i) = 0; }
        }

    return b;
}

// Riff: la sezione "ritornello", piu aperta e spinta
void DrumGenerator::applyRiff (Bar& b, const Pattern& p, const GenSettings& s, juce::Random& r)
{
    juce::ignoreUnused (s);
    const int L = b.steps;
    auto onBeat = [&] (int i) { return i % b.stepsPerBeat == 0; };
    auto offEighth = [&] (int i) { return b.grid == 4 ? (i % b.stepsPerBeat) == b.stepsPerBeat / 2 && b.stepsPerBeat >= 2 : (i % 3) == 2; };

    switch (p.style)
    {
        case SMetal: case SPunk:
            for (int i = 0; i < L; ++i)
            {
                const int hv = juce::jmax (b.get (HHClosed, i), b.get (Ride, i));
                b.at (HHClosed, i) = 0; b.at (Ride, i) = 0;
                if (onBeat (i)) b.at (Crash, i) = 105;
                else if (hv > 0 && p.style == SMetal) b.at (China, i) = 0;
            }
            break;

        case SFunk: case SHipHop: case SDisco: case SSoul:
            for (int i = 0; i < L; ++i)
                if (offEighth (i) && b.get (HHClosed, i) > 0) { b.at (HHOpen, i) = 95; b.at (HHClosed, i) = 0; }
            for (int i = 0; i < L; ++i)
                if (! onBeat (i) && b.get (Snare, i) == 0 && b.get (Kick, i) == 0 && chance (r, 0.15f)) b.at (Snare, i) = 42;
            break;

        case SElectronic:
            for (int i = 0; i < L; ++i)
            {
                if (b.get (Snare, i) >= 70) b.at (Clap, i) = 105;
                if (offEighth (i)) { b.at (HHOpen, i) = 95; b.at (HHClosed, i) = 0; }
                else if (b.grid == 4 && b.get (HHClosed, i) == 0) b.at (HHClosed, i) = 58;
            }
            break;

        case SReggae:
            for (int i = 0; i < L; i += b.stepsPerBeat) b.at (Kick, i) = juce::jmax (b.get (Kick, i), 100);
            break;

        case SLatin:
            for (int i = 0; i < L; ++i)
            {
                if (b.get (HHClosed, i) > 0) { b.at (Ride, i) = juce::jmax (85, b.get (HHClosed, i)); b.at (HHClosed, i) = 0; }
                if (onBeat (i)) b.at (RideBell, i) = 95;
            }
            break;

        case SJazz:
            for (int i = 0; i < L; ++i)
                if (! onBeat (i) && b.get (Snare, i) == 0 && chance (r, 0.18f)) b.at (Snare, i) = 50 + r.nextInt (20);
            setMax (b, Kick, L - 1, 85);
            break;

        default: // Rock, Pop, Blues, Country, Ballad, Motown
            if (b.uses (HHClosed))
            {
                for (int i = 0; i < L; ++i)
                {
                    if (b.get (HHClosed, i) > 0) { b.at (Ride, i) = juce::jmax (88, b.get (HHClosed, i)); b.at (HHClosed, i) = 0; }
                    if (b.get (HHOpen, i) > 0) { b.at (RideBell, i) = 100; b.at (HHOpen, i) = 0; }
                }
            }
            else if (b.uses (Ride))
            {
                for (int i = 0; i < L; ++i)
                    if (b.get (Ride, i) > 0 && onBeat (i)) b.at (RideBell, i) = 90;
            }
            if (b.grid == 4 && L > 6 && b.get (Kick, 6) == 0 && b.get (Snare, 6) == 0) b.at (Kick, 6) = 82;
            break;
    }
}

void DrumGenerator::applyFill (Bar& b, const Pattern& p, const GenSettings& s, int fillType, juce::Random& r)
{
    const bool den8 = (s.meterDen == 8);
    int beatSteps = den8 ? ((s.meterNum % 3 == 0) ? 6 : 4) : b.grid;
    int len = fillType == 1 ? beatSteps : (fillType == 2 ? 2 * beatSteps : juce::jmin (b.steps, 4 * beatSteps));
    if (fillType == 3 && b.steps <= 4 * beatSteps) len = b.steps;
    len = juce::jlimit (1, b.steps, len);
    const int start = b.steps - len;

    b.clearRange (start, b.steps);

    const int shape = (int) ((p.id + (juce::uint32) fillType * 3 + s.seed) % 4);
    const bool triplet = (b.grid == 3);

    auto velAt = [&] (int i) { return juce::jlimit (40, 127, 82 + (int) (40.0f * (float) (i - start) / (float) juce::jmax (1, len - 1))); };

    if (p.style == SElectronic)
    {
        for (int i = start; i < b.steps; ++i)
        {
            const bool firstHalf = (fillType == 3 && i < start + len / 2);
            if (firstHalf && ((i - start) % 2 != 0)) continue;
            b.at (Snare, i) = juce::jlimit (40, 127, 50 + (int) (75.0f * (float) (i - start) / (float) juce::jmax (1, len - 1)));
        }
        b.at (Clap, b.steps - 1) = 115;
        b.at (Kick, start) = 100;
        return;
    }

    const int toms[] = { Snare, TomHi, TomMid, TomFloor };
    for (int i = start; i < b.steps; ++i)
    {
        const int rel = i - start;
        const float prog = (float) rel / (float) juce::jmax (1, len);

        if (! triplet && fillType == 3 && rel < len / 2 && (rel % 2) != 0) continue;   // prima meta in crome

        if (p.style == SJazz && chance (r, 0.3f)) continue;
        if ((p.style == SFunk || p.style == SSoul || p.style == SHipHop) && chance (r, 0.25f))
        {
            b.at (Snare, i) = 45;
            continue;
        }

        int piece = Snare;
        switch (shape)
        {
            case 0: piece = toms[juce::jlimit (0, 3, (int) (prog * 4.0f))]; break;
            case 1: piece = (prog < 0.75f) ? (int) Snare : (int) TomFloor; break;
            case 2: piece = (rel % 2 == 0) ? (int) Snare : toms[1 + juce::jlimit (0, 2, (int) (prog * 3.0f))]; break;
            default:
            {
                const int cyc = rel % 4;
                piece = cyc == 0 ? (int) TomHi : cyc == 1 ? (int) TomMid : cyc == 2 ? (int) Kick : (int) TomFloor;
                break;
            }
        }
        if (p.style == SLatin) piece = (rel % 2 == 0) ? (int) TomHi : (int) TomMid;

        int v = velAt (i);
        if ((i % b.stepsPerBeat) == 0) v = juce::jmin (127, v + 8);
        b.at (piece, i) = v;

        if (p.style == SMetal && (rel % 2 == 0)) b.at (Kick, i) = 105;
        else if ((i % b.stepsPerBeat) == 0 && piece != Kick && chance (r, 0.5f)) b.at (Kick, i) = 95;
    }
    if (p.uses (HHPedal) || p.style == SJazz)
        for (int i = start; i < b.steps; i += b.stepsPerBeat * 2) b.at (HHPedal, i) = 70;
}

// Frecce: da -5 (essenziale) a +5 (virtuoso), effetti cumulativi
void DrumGenerator::applyLevel (Bar& b, const Pattern& p, const GenSettings& s, juce::Random& r)
{
    juce::ignoreUnused (p);
    const int lv = s.level;
    if (lv == 0) return;
    const int L = b.steps, sb = b.stepsPerBeat;
    auto isBeat = [&] (int i) { return i % sb == 0; };
    auto is8 = [&] (int i) { return b.grid == 4 ? (i % 2 == 0) : (i % 3 != 1); };
    const int hp = b.uses (HHClosed) ? (int) HHClosed : (b.uses (Ride) ? (int) Ride : -1);

    if (lv < 0)
    {
        for (int i = 0; i < L; ++i) if (b.get (Snare, i) > 0 && b.get (Snare, i) < 55) b.at (Snare, i) = 0;
        if (lv <= -2)
            for (int h : { (int) HHClosed, (int) HHOpen, (int) Ride, (int) RideBell, (int) Shaker, (int) Tambourine })
                for (int i = 0; i < L; ++i) if (! is8 (i)) b.at (h, i) = 0;
        if (lv <= -3)
        {
            for (int i = 0; i < L; ++i) if (! isBeat (i)) b.at (Kick, i) = 0;
            b.at (Kick, 0) = juce::jmax (b.get (Kick, 0), 100);
        }
        if (lv <= -4)
        {
            for (int h : { (int) HHClosed, (int) Ride })
                for (int i = 0; i < L; ++i) if (! isBeat (i)) b.at (h, i) = 0;
            for (int i = 0; i < L; ++i)
            {
                if (b.get (HHOpen, i) > 0 && isBeat (i) && hp >= 0) b.at (hp, i) = juce::jmax (b.get (hp, i), b.get (HHOpen, i));
                for (int pc : { (int) HHOpen, (int) RideBell, (int) Shaker, (int) Tambourine, (int) Cowbell }) b.at (pc, i) = 0;
            }
        }
        if (lv <= -5)
        {
            const bool hasSnare = b.uses (Snare);
            for (int i = 0; i < L; ++i)
            {
                if (b.get (Kick, i) > 0 && (i / sb) % 2 != 0) b.at (Kick, i) = 0;
                if (b.get (Snare, i) > 0 && b.get (Snare, i) < 90) b.at (Snare, i) = 0;
                if (b.get (Clap, i) > 0 && hasSnare) b.at (Clap, i) = 0;
            }
            b.at (Kick, 0) = juce::jmax (b.get (Kick, 0), 100);
        }
        return;
    }

    for (int i = 0; i < L; ++i)
    {
        if (isBeat (i) || b.get (Snare, i) > 0 || b.get (SideStick, i) > 0) continue;
        const bool near = (i > 0 && b.get (Snare, i - 1) >= 90) || (i + 1 < L && b.get (Snare, i + 1) >= 90);
        if (! near && chance (r, 0.22f + (lv >= 5 ? 0.25f : 0.0f))) b.at (Snare, i) = 38 + r.nextInt (12);
    }
    if (lv >= 2 && hp >= 0)
        for (int i = 0; i < L; ++i) if (b.get (hp, i) == 0 && b.get (HHOpen, i) == 0) b.at (hp, i) = 55;
    if (lv >= 3)
        for (int i = 0; i < L; ++i)
            if (! isBeat (i) && (is8 (i) || lv >= 5) && b.get (Kick, i) == 0 && b.get (Snare, i) < 90 && chance (r, 0.3f))
                b.at (Kick, i) = 80;
    if (lv >= 4)
    {
        const int o = L - juce::jmax (1, sb / 2);
        if (hp == HHClosed) { b.at (HHOpen, o) = 95; b.at (HHClosed, o) = 0; }
        for (int i = 0; i < L; ++i)
            if (hp == HHClosed && ! isBeat (i) && is8 (i) && b.get (HHClosed, i) > 0 && chance (r, 0.15f))
            { b.at (HHOpen, i) = 92; b.at (HHClosed, i) = 0; }
        if (chance (r, 0.5f))
            for (int i = L - sb; i < L; ++i)
            {
                if (i < 0) continue;
                if (i == L - 1 && b.grid == 4) b.at (TomHi, i) = 95;
                else b.at (Snare, i) = juce::jmax (b.get (Snare, i), 65 + r.nextInt (25));
            }
    }
    if (lv >= 5)
    {
        for (int i = 0; i < L - 1; ++i)
            if (b.get (Kick, i) >= 70 && b.get (Kick, i + 1) == 0 && b.get (Snare, i + 1) == 0 && chance (r, 0.5f))
                b.at (Kick, i + 1) = 75;
        if (b.uses (Ride))
            for (int i = 0; i < L; i += sb) b.at (RideBell, i) = 90;
    }
}

void DrumGenerator::applySpecial (Bar& b, const Pattern& p, const GenSettings& s, int special, juce::Random& r)
{
    juce::ignoreUnused (p, r);
    switch (special)
    {
        case StopBar:
        {
            const int half = b.steps / 2;
            b.clearRange (half, b.steps);
            const int beatSteps = (s.meterDen == 8) ? 2 : b.grid;
            for (int pos : { half, half + beatSteps + beatSteps / 2 })
                if (pos < b.steps)
                {
                    b.at (Crash, pos) = 118; b.at (Kick, pos) = 118; b.at (Snare, pos) = 112;
                }
            break;
        }
        case RollBar:
        {
            b.clearRange (0, b.steps);
            for (int i = 0; i < b.steps; ++i)
            {
                const float prog = (float) i / (float) juce::jmax (1, b.steps - 1);
                const bool lastBeat = i >= b.steps - b.stepsPerBeat;
                b.at (lastBeat ? (i % 2 == 0 ? (int) TomHi : (int) TomFloor) : (int) Snare, i) = juce::jlimit (30, 127, 35 + (int) (90.0f * prog));
                if (i % b.stepsPerBeat == 0) b.at (Kick, i) = 70 + (int) (40.0f * prog);
            }
            break;
        }
        case HitBar1: case HitBar2: case HitBar3:
            b.clearRange (0, b.steps, false);
            b.at (Crash, 0) = 122; b.at (Kick, 0) = 120;
            if (special == HitBar2) b.at (Snare, 0) = 115;
            if (special == HitBar3) { b.at (China, 0) = 110; b.at (TomFloor, 0) = 110; }
            break;
        case CountIn:
            b.clearRange (0, b.steps, false);
            for (int i = 0; i < b.steps; i += b.stepsPerBeat * ((s.meterDen == 8 && s.meterNum % 3 == 0) ? 3 : 1))
                b.at (SideStick, i) = (i == 0) ? 115 : 95;
            break;
        default: break;
    }
}

DrumGenerator::Bar DrumGenerator::makeBar (const Pattern& p, const GenSettings& s, int barIdx, bool riff, int fill,
                                           int special, bool crash, juce::Random& r)
{
    Bar b = groove (p, s, barIdx, r);
    if (riff) applyRiff (b, p, s, r);
    if (s.sideStick == 2 || (s.sideStick == 1 && ! riff))
        for (int i = 0; i < b.steps; ++i)
            if (b.get (Snare, i) >= 55) { b.at (SideStick, i) = juce::jmax (b.get (SideStick, i), b.get (Snare, i)); b.at (Snare, i) = 0; }
    if (special == None || special == StopBar) applyLevel (b, p, s, r);
    if (fill > 0) applyFill (b, p, s, fill, r);
    if (special != None) applySpecial (b, p, s, special, r);
    if (crash && s.crashOnOne && special != CountIn)
    {
        b.at (Crash, 0) = juce::jmax (b.get (Crash, 0), 115);
        b.at (Kick, 0) = juce::jmax (b.get (Kick, 0), 110);
        b.at (HHClosed, 0) = 0; b.at (HHOpen, 0) = 0; b.at (Ride, 0) = 0;
    }
    return b;
}

void DrumGenerator::emit (const Bar& b, double startPPQ, double scale, const GenSettings& s, juce::Random& r,
                          std::vector<DrumEvent>& out)
{
    // lo shuffle viene calcolato in emitBar e passato tramite s.swing / s.feelMode gia risolti
    const bool shuffle = (s.feelMode == 2);
    const float sw = juce::jlimit (0.5f, 0.75f, s.swing);

    for (int piece = 0; piece < NumPieces; ++piece)
    {
        if (s.mute[(size_t) groupOf (piece)]) continue;
        for (int st = 0; st < b.steps; ++st)
        {
            int v = b.get (piece, st);
            if (v <= 0) continue;

            double pos = st * b.stepPPQ;
            double q = std::floor (pos + 1e-9);
            double f = pos - q;

            if (b.grid == 3)
            {
                const int k = juce::roundToInt (f * 3.0);
                const double t = shuffle ? sw : 0.5;
                f = (k == 0) ? 0.0 : (k == 1 ? t * 0.5 : t);
            }
            else if (shuffle)
            {
                auto warp = [sw] (double x) { return x < 0.5 ? x * (sw / 0.5) : sw + (x - 0.5) * ((1.0 - sw) / 0.5); };
                if (! s.swing16) f = warp (f);
                else { const double h = std::floor (f * 2.0) / 2.0; f = h + warp ((f - h) * 2.0) / 2.0; }
            }
            pos = (q + f) * scale;

            float ms = s.feel * 30.0f + (r.nextFloat() + r.nextFloat() - 1.0f) * s.humTiming * 40.0f;

            const bool isGhost = (piece == Snare && v < 55);
            const bool timeKeeper = (piece == HHClosed || piece == Ride || piece == Shaker || piece == Tambourine);
            if (timeKeeper)
                v += (f < 1e-6) ? (int) (s.hatAccent * 26.0f) : -(int) (s.hatAccent * 34.0f);

            float fv = (float) v;
            fv *= isGhost ? (0.6f + 0.8f * s.ghost) : (0.62f + 0.6f * s.intensity);
            fv *= s.volume;
            fv += (r.nextFloat() * 2.0f - 1.0f) * s.humVelocity * 38.0f;

            DrumEvent e;
            e.ppq = startPPQ + pos;
            e.piece = piece;
            e.vel = juce::jlimit (1, 127, juce::roundToInt (fv));
            e.ms = ms;
            out.push_back (e);
        }
    }
}

void DrumGenerator::emitBar (const Pattern& p, const GenSettings& sIn, int barIdx, bool riff, int fill, int special,
                             bool crash, double startPPQ, juce::Random& r, std::vector<DrumEvent>& out)
{
    GenSettings s = sIn;
    bool shuffle = false;
    s.swing = effectiveSwing (p, sIn, shuffle);
    s.feelMode = shuffle ? 2 : 1;

    const double barQ = sIn.meterNum * 4.0 / (double) sIn.meterDen;

    if (s.timeFeel == 2)
    {
        // Double-time: due "mezze battute" per battuta
        const bool hit = (special == HitBar1 || special == HitBar2 || special == HitBar3 || special == CountIn);
        Bar a = makeBar (p, s, barIdx * 2, riff, 0, hit ? special : None, crash, r);
        emit (a, startPPQ, 0.5, s, r, out);
        if (! hit)
        {
            Bar c = makeBar (p, s, barIdx * 2 + 1, riff, fill, special, false, r);
            emit (c, startPPQ + barQ * 0.5, 0.5, s, r, out);
        }
        return;
    }

    Bar b = makeBar (p, s, barIdx, riff, fill, special, crash, r);
    emit (b, startPPQ, s.timeFeel == 1 ? 2.0 : 1.0, s, r, out);
}

DrumSequence DrumGenerator::renderPart (const Pattern& p, int part, const GenSettings& s)
{
    DrumSequence seq;
    seq.meterNum = s.meterNum; seq.meterDen = s.meterDen;
    juce::Random r (mixSeed (p.id, s.seed, (juce::uint32) part + 1));
    const double barLen = barLengthPPQ (s);
    const int N = juce::jmax (1, s.bars);

    if (part == PMain || part == PRiff || (part >= PFill1 && part <= PFill3))
    {
        for (int i = 0; i < N; ++i)
        {
            const bool riff = (part == PRiff);
            const int fill = (part >= PFill1 && part <= PFill3 && i == N - 1) ? part - PFill1 + 1 : 0;
            const bool crash = (i == 0) && (part != PMain);
            emitBar (p, s, i, riff, fill, None, crash, i * barLen, r, seq.events);
            seq.barKinds.push_back (fill > 0 ? 2 : (riff ? 1 : 0));
        }
        seq.numBars = N;
    }
    else
    {
        const int E = juce::jmax (2, N);
        const int endType = part - PEnd1 + 1;
        for (int i = 0; i < E; ++i)
        {
            int fill = 0, special = None;
            if (i == E - 1) special = endType == 1 ? HitBar1 : endType == 2 ? HitBar2 : HitBar3;
            else if (i == E - 2) { if (endType == 1) fill = 2; else special = endType == 2 ? StopBar : RollBar; }
            emitBar (p, s, i, false, fill, special, false, i * barLen, r, seq.events);
            seq.barKinds.push_back (i >= E - 2 ? 3 : 0);
        }
        seq.numBars = E;
    }

    seq.barPPQ = barLen;
    seq.lengthPPQ = barLen * seq.numBars;
    return seq;
}

DrumSequence DrumGenerator::renderSong (const Pattern& p, const GenSettings& s)
{
    DrumSequence seq;
    seq.meterNum = s.meterNum; seq.meterDen = s.meterDen;
    juce::Random r (mixSeed (p.id, s.seed, 999));
    const double barLen = barLengthPPQ (s);
    int bar = 0;

    if (s.countIn)
    {
        emitBar (p, s, 0, false, 0, CountIn, false, 0.0, r, seq.events);
        seq.barKinds.push_back (4);
        ++bar;
    }

    const int body = juce::jmax (1, s.songBars);
    const int section = juce::jmax (1, s.sectionBars);
    bool prevFill = false;

    for (int b = 0; b < body; ++b)
    {
        const int sec = b / section;
        const bool riff = s.altRiff && (sec % 2 == 1);
        int fill = 0;
        if (s.fillEvery > 0 && ((b + 1) % s.fillEvery == 0) && ! (s.ending > 0 && b == body - 1))
            fill = ((b + 1) % section == 0) ? 3 : ((((b + 1) / s.fillEvery) % 2) ? 1 : 2);
        const bool crash = (b == 0) || (b % section == 0) || prevFill;
        emitBar (p, s, b, riff, fill, None, crash, bar * barLen, r, seq.events);
        seq.barKinds.push_back (fill > 0 ? 2 : (riff ? 1 : 0));
        prevFill = fill > 0;
        ++bar;
    }

    if (s.ending > 0)
    {
        const int E = juce::jmax (2, juce::jmin (s.bars, 4));
        for (int i = 0; i < E; ++i)
        {
            int fill = 0, special = None;
            if (i == E - 1) special = s.ending == 1 ? HitBar1 : s.ending == 2 ? HitBar2 : HitBar3;
            else if (i == E - 2) { if (s.ending == 1) fill = 2; else special = s.ending == 2 ? StopBar : RollBar; }
            emitBar (p, s, body + i, false, fill, special, i == 0 && prevFill, bar * barLen, r, seq.events);
            seq.barKinds.push_back (3);
            ++bar;
        }
    }

    seq.numBars = bar;
    seq.barPPQ = barLen;
    seq.lengthPPQ = barLen * bar;
    return seq;
}

juce::MidiFile DrumGenerator::toMidiFile (const DrumSequence& seq, const std::array<int, NumPieces>& noteMap,
                                          int channel, double bpm, int num, int den, const juce::String& name)
{
    constexpr int tpq = 960;
    juce::MidiFile mf;
    mf.setTicksPerQuarterNote (tpq);
    juce::MidiMessageSequence track;

    auto meta = juce::MidiMessage::textMetaEvent (3, name);
    meta.setTimeStamp (0);
    track.addEvent (meta);
    auto tempo = juce::MidiMessage::tempoMetaEvent (juce::roundToInt (60000000.0 / juce::jmax (20.0, bpm)));
    tempo.setTimeStamp (0);
    track.addEvent (tempo);
    int denPow = 2;
    for (int d = den, k = 0; d > 1; d >>= 1, ++k) denPow = k + 1;
    juce::ignoreUnused (denPow);
    auto ts = juce::MidiMessage::timeSignatureMetaEvent (num, den);
    ts.setTimeStamp (0);
    track.addEvent (ts);

    const int ch = juce::jlimit (1, 16, channel);
    for (auto& e : seq.events)
    {
        const double t = juce::jmax (0.0, (e.ppq + e.ms * bpm / 60000.0) * tpq);
        const int note = juce::jlimit (0, 127, noteMap[(size_t) e.piece]);
        auto on = juce::MidiMessage::noteOn (ch, note, (juce::uint8) juce::jlimit (1, 127, e.vel));
        on.setTimeStamp (t);
        auto off = juce::MidiMessage::noteOff (ch, note);
        off.setTimeStamp (t + tpq / 8);
        track.addEvent (on);
        track.addEvent (off);
    }
    auto eot = juce::MidiMessage::endOfTrack();
    eot.setTimeStamp (seq.lengthPPQ * tpq);
    track.addEvent (eot);
    track.updateMatchedPairs();
    mf.addTrack (track);
    return mf;
}
} // namespace may
