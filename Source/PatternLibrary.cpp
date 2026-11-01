#include "PatternLibrary.h"

namespace may
{
namespace
{
struct Core { const char* name; int grid; const char* data; };

// Notazione: una riga per strumento, uno step per carattere (sedicesimi o terzine).
// X accento, x normale, o leggero, g ghost, - pausa.
// K cassa, S rullante, X sidestick, H hh chiuso, O hh aperto, P hh pedale, R ride, B campana,
// C crash, 1 2 3 tom, L clap, A tamburello, Q campanaccio, M shaker, Y china.
const std::vector<std::vector<Core>>& cores()
{
    static const std::vector<std::vector<Core>> c =
    {
        { // Rock
            { "Classic Rock",   4, "K:x-------x-x-----|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Arena Rock",   4, "K:x---x---x---x---|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Hard Rock",    4, "K:x-----x-x-x-----|S:----X-------X--g|H:X-x-X-x-X-x-X-x-" },
            { "Rock 16",   4, "K:x-------x-x---x-|S:----X--g----X---|H:XoxoXoxoXoxoXoxo" },
        },
        { // Pop
            { "Pop Drive",    4, "K:x-------x-------|S:----X-------X---|H:XoxoXoxoXoxoXoxo" },
            { "Pop Bounce",   4, "K:x--x--x---x-----|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Dance Pop",     4, "K:x-------x-x-----|S:----X-------X---|L:----x-------x---|H:X-x-X-x-X-x-X---|O:--------------x-" },
            { "Acoustic Pop",   4, "K:x-----x---x--x--|S:----X-------X---|M:xoxoxoxoxoxoxoxo|H:x---x---x---x---" },
        },
        { // Funk
            { "Funk 16",      4, "K:x-x-------x--x--|S:----X--g-g--X--g|H:XxxxXxxxXxxxXxxx" },
            { "Break Funk",   4, "K:x-x-----x-x--x--|S:----X--g-g-gX--g|H:xxxxxxxxxxx-xxxx|O:-----------x----" },
            { "Syncopated Funk",   4, "K:x--x--x---x--x--|S:----X-g-----X-g-|H:XoXoXoXoXoXoXoXo" },
            { "Linear Funk",  4, "K:x------xx-x-----|S:----X--g-g--X---|H:x-x-x-x-x-x-x-x-|O:--------------x-" },
        },
        { // Blues (terzine)
            { "Chicago Shuffle",      3, "K:x-----x-----|S:---X-----X--|H:x-xx-xx-xx-x" },
            { "Texas Shuffle",3, "K:x--x--x--x--|S:---X-----X--|R:x-xx-xx-xx-x" },
            { "Slow Blues 12-8",    3, "K:x-----x--x--|S:---X-----X--|H:XooXooXooXoo" },
            { "Shuffle Ghost",3, "K:x----xx-----|S:-g-X-g-g-X-g|H:x-xx-xx-xx-x" },
        },
        { // Country
            { "Train Beat",   4, "K:x-------x-------|S:gxgXgxgXgxgXgxgX|P:----x-------x---" },
            { "Two Step",     4, "K:x-------x-------|X:----x-------x---|H:x-x-x-x-x-x-x-x-" },
            { "Country Rock", 4, "K:x-----x-x-------|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Bluegrass",   4, "K:x-------x-------|S:----X-------X---|H:----x-------x---|M:x-x-x-x-x-x-x-x-" },
        },
        { // Jazz (terzine)
            { "Medium Swing",   3, "R:X--x-xX--x-x|P:---x-----x--|K:g--g--g--g--|S:--------g---" },
            { "Ballad Swing", 3, "R:x--x-xx--x-x|P:---x-----x--|S:g-g--g---g-g|K:g-----g-----" },
            { "Bebop",        3, "R:x--x-xx--x-x|P:---x-----x--|S:-----g--g---|K:-----------o" },
            { "Two Feel",     3, "R:x--x--x--x--|P:---x-----x--|K:o-----o-----|S:--------g--g" },
        },
        { // Metal
            { "Double Kick",  4, "K:xxxxxxxxxxxxxxxx|S:----X-------X---|C:x---x---x---x---" },
            { "Thrash",       4, "K:x---x---x---x---|S:--X---X---X---X-|R:X-x-X-x-X-x-X-x-" },
            { "Groove Metal", 4, "K:xx------xx-x----|S:--------X-------|H:X-x-X-x-X-x-X-x-" },
            { "Blast Beat",        4, "K:x-x-x-x-x-x-x-x-|S:-x-x-x-x-x-x-x-x|R:x-x-x-x-x-x-x-x-" },
        },
        { // Punk
            { "Punk Rock",       4, "K:x-x---x-x-x---x-|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Skate Punk",   4, "K:x---x---x---x---|S:--X---X---X---X-|H:x-x-x-x-x-x-x-x-" },
            { "D-Beat",       4, "K:x--x--x-x--x----|S:----X-------X---|C:x-x-x-x-x-x-x-x-" },
            { "Pop Punk",     4, "K:x-x-x-x-x-x-x-x-|S:----X-------X---|R:X-x-X-x-X-x-X-x-" },
        },
        { // Reggae
            { "One Drop",     4, "K:--------x-------|X:--------X-------|H:x-x-x-x-x-x-x-x-" },
            { "Rockers",      4, "K:x-------x-------|S:----X-------X---|H:-xx--xx--xx--xx-" },
            { "Steppers",     4, "K:x---x---x---x---|X:----x-------x---|H:--x---x---x---x-" },
            { "Roots",4, "K:--------x-------|S:--------X-------|H:x-x-x-x-x-x-x---|O:--------------x-" },
        },
        { // Hip Hop
            { "Boom Bap",     4, "K:x------x-x------|S:----X-------X---|H:x-x-x-x-x-x-x-x-" },
            { "Bap 16",       4, "K:x-x----x--x-----|S:----X-------X--g|H:XoxoXoxoXoxoXoxo" },
            { "Trap",    4, "K:x-----x---x-----|S:--------X-------|H:xxxxxxxxxxxxxxxx" },
            { "Lo-Fi",    4, "K:x-----x---x-x---|S:----X-------X---|L:----x-------x---|H:x-x-x-x-x-x-x-x-" },
        },
        { // R&B / Soul
            { "Soul",  4, "K:x-----x-x-------|S:----X--g----X---|H:x-x-x-x-x-x-x-x-" },
            { "Neo Soul",     4, "K:x--x------x--x--|S:----X---g---X-g-|H:oooooooooooooooo" },
            { "Classic R&B",    4, "K:x-------x-x-----|X:----x-------x---|H:x-x-x-x-x-x-x-x-" },
            { "Ghost Groove", 4, "K:x-------x-x-----|S:-g-gX-g--g-gX-g-|H:x-x-x-x-x-x-x-x-" },
        },
        { // Disco
            { "Disco",   4, "K:x---x---x---x---|S:----X-------X---|H:x---x---x---x---|O:--x---x---x---x-" },
            { "Disco 16",     4, "K:x---x---x---x---|S:----X-------X---|H:xxxxxx-xxxxxxx-x|O:------x-------x-" },
            { "Nu Disco",   4, "K:x---x---x---x---|S:----X-------X---|L:----x-------x---|O:--x---x---x---x-|A:xxxxxxxxxxxxxxxx" },
            { "Cowbell Disco",4, "K:x---x---x---x---|S:----X--g----X---|H:x-x-x-x-x-x-x-x-|Q:x--x--x---x--x--" },
        },
        { // Latin
            { "Bossa Nova",   4, "K:x--xx--xx--xx--x|X:x--x--x---x--x--|H:x-x-x-x-x-x-x-x-" },
            { "Samba",        4, "K:o--Xo--Xo--Xo--X|X:x-x--x-x-x-x--x-|H:XxxxXxxxXxxxXxxx" },
            { "Cha Cha",      4, "K:x-------x-------|Q:X---x---X---x---|X:------x-------x-|3:------------x-x-" },
            { "Songo",        4, "K:---x------x---x-|S:--g---x-g---x---|Q:X-x-X-x-x-x-X-x-|3:------x-------x-" },
        },
        { // Ballad
            { "Ballad",       4, "K:x-------x-------|S:----x-------x---|H:o-o-o-o-o-o-o-o-" },
            { "Rim Ballad",   4, "K:x-----x-x-------|X:----x-------x---|H:o-o-o-o-o-o-o-o-" },
            { "Power Ballad", 4, "K:x-------x-x-----|S:----X-------X---|R:x-x-x-x-x-x-x-x-" },
            { "Half Time Ballad",  4, "K:x-----x---------|S:--------X-------|H:o-o-o-o-o-o-o-o-" },
        },
        { // Motown
            { "Motown",  4, "K:x-----x-x-----x-|S:x---X---x---X---|H:x-x-x-x-x-x-x-x-|A:----x-------x---" },
            { "Northern Soul",4, "K:x-x---x-x-x---x-|S:----X-------X---|A:----x-------x---|H:x-x-x-x-x-x-x-x-" },
            { "Tamb 16",      4, "K:x---x---x---x---|S:----X-------X---|A:xxxxxxxxxxxxxxxx" },
            { "Soul Stomp",   4, "K:x---x---x---x---|S:----X-------X---|L:----x-------x---|H:x-x-x-x-x-x-x-x-" },
        },
        { // Electronic
            { "House",        4, "K:x---x---x---x---|L:----x-------x---|H:x-x-x-x-x-x-x-x-|O:--x---x---x---x-" },
            { "Techno",       4, "K:x---x---x---x---|S:----o-------o---|H:--x---x---x---x-|R:x-x-x-x-x-x-x-x-" },
            { "Breakbeat",    4, "K:x-x-------x-----|S:----X--g-g--X---|H:x-x-x-x-x-x-x-x-" },
            { "Drum & Bass",  4, "K:x---------x-----|S:----X-------X---|H:x-x-x-x-x-x-x-x-" },
        },
    };
    return c;
}

void moveLine (Pattern& p, int from, int to, int minVel)
{
    auto& f = p.lines[(size_t) from];
    if (f.empty()) return;
    auto& t = p.lines[(size_t) to];
    if (t.empty()) t.assign (f.size(), 0);
    for (size_t i = 0; i < f.size(); ++i)
        if (f[i] > 0) { t[i] = (juce::uint8) juce::jmax ((int) f[i], minVel); f[i] = 0; }
}

void setIfEmpty (Pattern& p, int piece, int step, int vel)
{
    auto& l = p.lines[(size_t) piece];
    if (l.empty()) l.assign ((size_t) (p.stepsPerBar * p.coreBars), 0);
    for (int b = 0; b < p.coreBars; ++b)
    {
        auto i = (size_t) (b * p.stepsPerBar + step);
        if (i < l.size() && l[i] == 0) l[i] = (juce::uint8) vel;
    }
}

bool stepUsed (const Pattern& p, int piece, int step)
{
    auto& l = p.lines[(size_t) piece];
    return ! l.empty() && l[(size_t) step] > 0;
}
} // namespace

const char* PatternLibrary::styleName (int s)
{
    static const char* n[] = { "Rock", "Pop", "Funk", "Blues", "Country", "Jazz", "Metal", "Punk",
                               "Reggae", "Hip Hop", "R&B Soul", "Disco", "Latin", "Ballad", "Motown", "Electronic" };
    return n[juce::jlimit (0, NumStyles - 1, s)];
}

juce::StringArray PatternLibrary::styleNames()
{
    juce::StringArray a;
    for (int i = 0; i < NumStyles; ++i) a.add (styleName (i));
    return a;
}

const PatternLibrary& PatternLibrary::get()
{
    static PatternLibrary lib;
    return lib;
}

juce::StringArray PatternLibrary::subNames (int style) const
{
    return subs[(size_t) juce::jlimit (0, NumStyles - 1, style)];
}

juce::StringArray PatternLibrary::presetNames (int style, int sub) const
{
    juce::StringArray a;
    for (int i = 0; i < presetsPerSub; ++i) a.add (getPattern (style, sub, i).name);
    return a;
}

const Pattern& PatternLibrary::getPattern (int style, int sub, int preset) const
{
    style  = juce::jlimit (0, NumStyles - 1, style);
    sub    = juce::jlimit (0, subsPerStyle - 1, sub);
    preset = juce::jlimit (0, presetsPerSub - 1, preset);
    return patterns[(size_t) style][(size_t) sub][(size_t) preset];
}

Pattern PatternLibrary::makeAlternative (const Pattern& p, juce::uint32 alt) const
{
    if (alt == 0) return p;
    juce::Random r ((juce::int64) (p.id * 2654435761u ^ alt * 40503u ^ 77u));
    std::vector<const Pattern*> donors;
    for (int sb = 0; sb < subsPerStyle; ++sb)
        for (int pr = 0; pr < presetsPerSub; ++pr)
        {
            const auto& d = patterns[(size_t) p.style][(size_t) sb][(size_t) pr];
            if (d.grid == p.grid && d.id != p.id) donors.push_back (&d);
        }
    Pattern q = p;
    q.id = p.id ^ (alt * 7919u);
    if (donors.empty()) return q;
    auto pick = [&] { return donors[(size_t) r.nextInt ((int) donors.size())]; };

    auto take = [&] (const Pattern& d, std::initializer_list<int> pieces)
    {
        const int bars = juce::jmax (q.coreBars, d.coreBars);
        if (bars > q.coreBars)
        {
            for (auto& l : q.lines)
                if (! l.empty() && (int) l.size() < bars * q.stepsPerBar)
                {
                    std::vector<juce::uint8> o;
                    for (int b = 0; b < bars; ++b) o.insert (o.end(), l.begin(), l.begin() + q.stepsPerBar);
                    l = o;
                }
            q.coreBars = bars;
        }
        for (int pc : pieces)
        {
            auto& out = q.lines[(size_t) pc];
            out.clear();
            if (! d.uses (pc)) continue;
            for (int b = 0; b < bars; ++b)
                for (int i = 0; i < q.stepsPerBar; ++i)
                    out.push_back ((juce::uint8) d.at (pc, b, i));
        }
    };

    const auto* kd = pick();
    const auto* hd = pick();
    if (r.nextFloat() < 0.85f) take (*kd, { Kick });
    if (r.nextFloat() < 0.85f) take (*hd, { HHClosed, HHOpen, Ride, RideBell, HHPedal });
    if (r.nextFloat() < 0.5f)  take (*pick(), { Shaker, Tambourine, Cowbell });

    auto& k = q.lines[Kick];
    auto& sn = q.lines[Snare];
    if (! k.empty() && ! sn.empty())
        for (size_t i = 0; i < k.size() && i < sn.size(); ++i)
            if (k[i] > 0 && sn[i] >= 90 && (int) i % q.grid != 0) k[i] = 0;

    q.name = p.name + " Alt " + juce::String (alt);
    q.fullName = p.fullName + " - Alternativo " + juce::String (alt);
    return q;
}

namespace
{
// Variante 1: hi-hat aperti, ride, comping jazz o shaker a seconda del genere
juce::String applyVariantColour (Pattern& p)
{
    const int s = p.style;
    if (s == SFunk || s == SHipHop || s == SDisco || s == SElectronic || s == SSoul)
    {
        auto& h = p.lines[HHClosed];
        bool done = false;
        if (! h.empty())
            for (int i = 6; i < (int) h.size(); i += 8)
                if (h[(size_t) i] > 0) { setIfEmpty (p, HHOpen, i % p.stepsPerBar, 90); h[(size_t) i] = 0; done = true; }
        if (! done) for (int i = 2; i < p.stepsPerBar; i += 4) setIfEmpty (p, HHOpen, i, 85);
        return "Open Hat";
    }
    if (s == SJazz)
    {
        setIfEmpty (p, Snare, 5, 45);
        setIfEmpty (p, Snare, 8, 40);
        setIfEmpty (p, Kick, 11, 70);
        return "Comping";
    }
    if (p.uses (HHClosed))
    {
        moveLine (p, HHClosed, Ride, 88);
        moveLine (p, HHOpen, RideBell, 95);
        return "Ride";
    }
    if (p.uses (Ride))
    {
        moveLine (p, Ride, HHClosed, 80);
        return "Hi-Hat";
    }
    for (int i = 0; i < p.stepsPerBar; i += (p.grid == 4 ? 2 : 1))
        setIfEmpty (p, Shaker, i, (i % p.grid == 0) ? 95 : 70);
    return "Shaker";
}

// Variante "Plus": hi-hat piu fitti, ghost notes, spinta di cassa
void applyVariantPlus (Pattern& p)
{
    const int s = p.style;
    if (p.grid == 4)
    {
        const bool heavy = (s == SMetal || s == SPunk || s == SReggae);
        int hatPiece = p.uses (HHClosed) ? (int) HHClosed : (p.uses (Ride) ? (int) Ride : -1);
        if (hatPiece >= 0 && ! heavy)
        {
            int count = 0;
            for (int i = 0; i < 16; ++i) if (stepUsed (p, hatPiece, i)) ++count;
            if (count <= 8)
                for (int i = 1; i < 16; i += 2)
                    if (! stepUsed (p, HHOpen, i)) setIfEmpty (p, hatPiece, i, 62);
        }
        if (! stepUsed (p, Snare, 6) && ! stepUsed (p, Snare, 5)) setIfEmpty (p, Snare, 7, 40);
        if (! stepUsed (p, Snare, 14)) setIfEmpty (p, Snare, 15, 40);
        if (s != SElectronic && s != SReggae && s != SDisco) setIfEmpty (p, Kick, 14, 85);
    }
    else
    {
        setIfEmpty (p, Snare, 2, 40);
        setIfEmpty (p, Snare, 8, 42);
        setIfEmpty (p, Kick, 11, 75);
    }
}

// Variante "Sync": cassa sincopata
void applyVariantSync (Pattern& p)
{
    if (p.grid == 4)
    {
        if (! stepUsed (p, Snare, 6) && ! stepUsed (p, SideStick, 6)) setIfEmpty (p, Kick, 6, 88);
        if (! stepUsed (p, Snare, 10)) setIfEmpty (p, Kick, 10, 85);
        if (! stepUsed (p, Snare, 3)) setIfEmpty (p, Kick, 3, 70);
    }
    else
    {
        setIfEmpty (p, Kick, 5, 80);
        setIfEmpty (p, Kick, 11, 80);
    }
}
} // namespace

PatternLibrary::PatternLibrary()
{
    auto& all = cores();
    patterns.resize ((size_t) NumStyles);
    subs.resize ((size_t) NumStyles);

    for (int s = 0; s < NumStyles; ++s)
    {
        auto& styleCores = all[(size_t) s];
        patterns[(size_t) s].resize ((size_t) subsPerStyle);

        for (int sub = 0; sub < subsPerStyle; ++sub)
        {
            const auto& c = styleCores[(size_t) (sub % (int) styleCores.size())];
            subs[(size_t) s].add (c.name);

            for (int v = 0; v < presetsPerSub; ++v)
            {
                Pattern p;
                p.style = s;
                p.sub = sub;
                p.grid = c.grid;
                p.stepsPerBar = c.grid * 4;
                p.id = (juce::uint32) (s * 1000 + sub * 100 + v * 7 + 11);

                juce::StringArray rows;
                rows.addTokens (c.data, "|", "");
                int maxLen = 0;
                for (auto& r : rows)
                {
                    if (r.length() < 3) continue;
                    int piece = pieceFromChar ((char) r[0]);
                    if (piece < 0) continue;
                    auto steps = r.substring (2);
                    auto& line = p.lines[(size_t) piece];
                    line.clear();
                    for (int i = 0; i < steps.length(); ++i)
                        line.push_back ((juce::uint8) velFromChar ((char) steps[i]));
                    maxLen = juce::jmax (maxLen, (int) line.size());
                }
                p.coreBars = juce::jmax (1, maxLen / p.stepsPerBar);
                for (auto& l : p.lines)
                    if (! l.empty()) l.resize ((size_t) (p.coreBars * p.stepsPerBar), 0);

                juce::String label;
                switch (v)
                {
                    case 0: label = "Base"; break;
                    case 1: label = applyVariantColour (p); break;
                    case 2: applyVariantPlus (p); label = "Plus"; break;
                    case 3: label = applyVariantColour (p) + " Plus"; applyVariantPlus (p); break;
                    default: applyVariantSync (p); label = "Sync"; break;
                }

                p.name = juce::String (v + 1).paddedLeft ('0', 2) + "  " + label;
                p.fullName = juce::String (styleName (s)) + " / " + c.name + " / " + label;
                patterns[(size_t) s][(size_t) sub].push_back (std::move (p));
            }
        }
    }
}
} // namespace may
