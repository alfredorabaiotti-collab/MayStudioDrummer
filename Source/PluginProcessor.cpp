#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace may;

namespace
{
const char* paramIds[] = {
    "genre", "sub", "preset", "part", "meterHost", "meter", "bpmHost", "bpm", "bars", "feelMode", "swing", "swing16",
    "timeFeel", "mdX", "mdY", "humTiming", "humVel", "volume", "feel", "ghost", "hatAccent", "cymbals", "crash", "seed",
    "playMode", "followHost", "preview", "channel", "songBars", "sectionBars", "fillEvery", "ending", "countIn", "altRiff",
    "level", "sideStick", "alt", "kitOn", "kitVol", "mute0", "mute1", "mute2", "mute3", "mute4", "mute5"
};

struct MeterDef { const char* name; int num, den; };
const MeterDef meters[] = { { "4/4", 4, 4 }, { "3/4", 3, 4 }, { "2/4", 2, 4 }, { "5/4", 5, 4 }, { "6/4", 6, 4 },
                            { "7/4", 7, 4 }, { "5/8", 5, 8 }, { "6/8", 6, 8 }, { "7/8", 7, 8 }, { "9/8", 9, 8 },
                            { "12/8", 12, 8 } };
} // namespace

juce::StringArray MayDrummerProcessor::meterNames()
{
    juce::StringArray a;
    for (auto& m : meters) a.add (m.name);
    return a;
}

juce::AudioProcessorValueTreeState::ParameterLayout MayDrummerProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    auto pct = [] (const String& id, const String& name, float def, float lo = 0.0f, float hi = 100.0f)
    { return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, NormalisableRange<float> (lo, hi, 1.0f), def, AudioParameterFloatAttributes().withLabel ("%")); };
    auto choice = [] (const String& id, const String& name, StringArray c, int def)
    { return std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, c, def); };
    auto boolean = [] (const String& id, const String& name, bool def)
    { return std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def); };
    auto integer = [] (const String& id, const String& name, int lo, int hi, int def)
    { return std::make_unique<AudioParameterInt> (ParameterID { id, 1 }, name, lo, hi, def); };

    StringArray parts;
    for (int i = 0; i < NumParts; ++i) parts.add (partName (i));

    l.add (choice ("genre", "Genere", PatternLibrary::styleNames(), 0));
    l.add (integer ("sub", "Sottogenere", 0, PatternLibrary::subsPerStyle - 1, 0));
    l.add (integer ("preset", "Preset", 0, PatternLibrary::presetsPerSub - 1, 0));
    l.add (choice ("part", "Parte", parts, 0));
    l.add (boolean ("meterHost", "Misura da progetto", true));
    l.add (choice ("meter", "Misura", meterNames(), 0));
    l.add (boolean ("bpmHost", "BPM da progetto", true));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "bpm", 1 }, "BPM", NormalisableRange<float> (40.0f, 240.0f, 1.0f), 120.0f));
    l.add (choice ("bars", "Battute", StringArray { "1", "2", "4", "8" }, 1));
    l.add (choice ("feelMode", "Feel", StringArray { "Auto", "Dritto", "Shuffle" }, 0));
    l.add (pct ("swing", "Swing", 50.0f, 50.0f, 75.0f));
    l.add (choice ("swing16", "Griglia swing", StringArray { "1/8", "1/16" }, 0));
    l.add (choice ("timeFeel", "Tempo", StringArray { "Normale", "Half-time", "Double-time" }, 0));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "mdX", 1 }, juce::String::fromUTF8 ("May Drummer: Imprevedibilità"), NormalisableRange<float> (0.0f, 1.0f), 0.25f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "mdY", 1 }, "May Drummer: Forza", NormalisableRange<float> (0.0f, 1.0f), 0.6f));
    l.add (pct ("humTiming", "Humanize Timing", 15.0f));
    l.add (pct ("humVel", "Humanize Velocity", 15.0f));
    l.add (pct ("volume", "Volume", 100.0f, 0.0f, 150.0f));
    l.add (pct ("feel", "Push / Laid back", 0.0f, -100.0f, 100.0f));
    l.add (pct ("ghost", "Ghost notes", 50.0f));
    l.add (pct ("hatAccent", "Accento piatti", 50.0f));
    l.add (choice ("cymbals", "Piatti", StringArray { "Auto", "Hi-Hat", "Ride" }, 0));
    l.add (boolean ("crash", "Crash su 1", true));
    l.add (integer ("level", "Livello complessita", -5, 5, 0));
    l.add (choice ("sideStick", "Rullante", StringArray { "Normale", "Side stick Main", "Side stick sempre" }, 0));
    l.add (integer ("alt", "Ritmo alternativo", 0, 99999, 0));
    l.add (boolean ("kitOn", "Suoni interni", true));
    l.add (pct ("kitVol", "Volume kit interno", 70.0f));
    l.add (integer ("seed", "Seed", 1, 9999, 1));
    l.add (choice ("playMode", "Riproduzione", StringArray { "Loop parte", "Song" }, 0));
    l.add (boolean ("followHost", "Segui trasporto DAW", true));
    l.add (boolean ("preview", "Anteprima interna", false));
    l.add (integer ("channel", "Canale MIDI", 1, 16, 10));
    l.add (integer ("songBars", "Battute song", 4, 128, 32));
    l.add (choice ("sectionBars", "Sezione", StringArray { "4", "8", "16" }, 1));
    l.add (choice ("fillEvery", "Fill", StringArray { "Mai", "Ogni 2", "Ogni 4", "Ogni 8" }, 2));
    l.add (choice ("ending", "Finale song", StringArray { "Nessuno", "Finale 1", "Finale 2", "Finale 3" }, 1));
    l.add (boolean ("countIn", "Count-in", false));
    l.add (boolean ("altRiff", "Alterna Main/Riff", true));
    const char* groups[] = { "Mute Cassa", "Mute Rullante", "Mute Hi-Hat", "Mute Tom", "Mute Piatti", "Mute Percussioni" };
    for (int i = 0; i < NumGroups; ++i) l.add (boolean ("mute" + String (i), groups[i], false));
    return l;
}

MayDrummerProcessor::MayDrummerProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MayDrummer", createLayout())
{
    resetNoteMap();
    loadFavourites();
    // pulizia delle cartelle temporanee dei trascinamenti precedenti (piu vecchie di un giorno)
    {
        auto root = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("MayStudioMidiDrummer");
        for (auto& d : root.findChildFiles (juce::File::findDirectories, false))
            if (d.getLastModificationTime() < juce::Time::getCurrentTime() - juce::RelativeTime::days (1))
                d.deleteRecursively();
    }
    for (auto* id : paramIds) apvts.addParameterListener (id, this);
    regenerateNow();
    startTimerHz (5);
}

MayDrummerProcessor::~MayDrummerProcessor()
{
    stopTimer();
    for (auto* id : paramIds) apvts.removeParameterListener (id, this);
}

void MayDrummerProcessor::resetNoteMap()
{
    for (int i = 0; i < NumPieces; ++i) noteMap[(size_t) i] = gmNote (i);
    mapChanged = true;
    generation++;
}

void MayDrummerProcessor::setNote (int piece, int note)
{
    const int n = juce::jlimit (0, 127, note);
    if (noteMap[(size_t) piece].load() == n) return;
    noteMap[(size_t) piece] = n;
    mapChanged = true;      // il thread audio spegne le note rimaste accese con la vecchia mappa
    generation++;           // l'interfaccia ridisegna parti e anteprime
}

void MayDrummerProcessor::setParam (const juce::String& id, float plainValue)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (plainValue));
}

const Pattern& MayDrummerProcessor::currentPattern() const
{
    const auto& base = PatternLibrary::get().getPattern ((int) getParam ("genre"), (int) getParam ("sub"), (int) getParam ("preset"));
    const int alt = (int) getParam ("alt");
    if (alt <= 0) return base;
    const auto key = base.fullName + "#" + juce::String (alt);
    if (key != altKey)
    {
        altPattern = PatternLibrary::get().makeAlternative (base, (juce::uint32) alt);
        altKey = key;
    }
    return altPattern;
}

juce::File MayDrummerProcessor::favouritesFile() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("MayStudioMidiDrummer").getChildFile ("preferiti.txt");
}

void MayDrummerProcessor::loadFavourites()
{
    favourites.clear();
    juce::StringArray lines;
    lines.addLines (favouritesFile().loadFileAsString());
    for (auto& l : lines) if (l.trim().isNotEmpty()) favourites.insert (l.trim());
}

void MayDrummerProcessor::toggleFavourite (int g, int s, int p)
{
    const auto k = favKey (g, s, p);
    if (favourites.count (k)) favourites.erase (k); else favourites.insert (k);
    juce::StringArray lines;
    for (auto& f : favourites) lines.add (f);
    auto f = favouritesFile();
    f.getParentDirectory().createDirectory();
    f.replaceWithText (lines.joinIntoString ("\n"));
    generation++;
}

juce::File MayDrummerProcessor::userPresetFolder() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("MayStudioMidiDrummer").getChildFile ("Impostazioni");
}

static juce::String safePresetName (const juce::String& n)
{
    return n.trim().removeCharacters ("\\/:*?\"<>|").substring (0, 60);
}

juce::StringArray MayDrummerProcessor::listUserPresets() const
{
    juce::StringArray names;
    for (auto& f : userPresetFolder().findChildFiles (juce::File::findFiles, false, "*.maydrummer"))
        names.add (f.getFileNameWithoutExtension());
    names.sortNatural();
    return names;
}

bool MayDrummerProcessor::saveUserPreset (const juce::String& rawName)
{
    const auto name = safePresetName (rawName);
    if (name.isEmpty()) return false;
    auto state = apvts.copyState();
    auto map = state.getOrCreateChildWithName ("NoteMap", nullptr);
    for (int i = 0; i < NumPieces; ++i) map.setProperty ("n" + juce::String (i), noteMap[(size_t) i].load(), nullptr);
    state.setProperty ("presetName", name, nullptr);
    auto xml = state.createXml();
    if (xml == nullptr) return false;
    auto dir = userPresetFolder();
    dir.createDirectory();
    const bool ok = xml->writeTo (dir.getChildFile (name + ".maydrummer"));
    if (ok) { currentUserPreset = name; generation++; }
    return ok;
}

bool MayDrummerProcessor::loadUserPreset (const juce::String& name)
{
    auto xml = juce::XmlDocument::parse (userPresetFolder().getChildFile (safePresetName (name) + ".maydrummer"));
    if (xml == nullptr) return false;
    auto state = juce::ValueTree::fromXml (*xml);
    if (! state.hasType (apvts.state.getType())) return false;

    const float previewWas = getParam ("preview");
    auto map = state.getChildWithName ("NoteMap");
    if (map.isValid())
        for (int i = 0; i < NumPieces; ++i)
            noteMap[(size_t) i] = (int) map.getProperty ("n" + juce::String (i), gmNote (i));
    mapChanged = true;
    lastPatternKey = {};   // non azzerare il ritmo alternativo salvato
    apvts.replaceState (state);
    setParam ("preview", previewWas);
    currentUserPreset = safePresetName (name);
    regenerateNow();
    return true;
}

bool MayDrummerProcessor::deleteUserPreset (const juce::String& name)
{
    auto f = userPresetFolder().getChildFile (safePresetName (name) + ".maydrummer");
    const bool ok = f.moveToTrash() || f.deleteFile();
    if (ok && currentUserPreset == name) currentUserPreset = {};
    generation++;
    return ok;
}

void MayDrummerProcessor::loadPattern (int g, int s, int p)
{
    setParam ("genre", (float) g);
    setParam ("sub", (float) s);
    setParam ("preset", (float) p);
    setParam ("alt", 0.0f);
}

double MayDrummerProcessor::effectiveBpm() const
{
    const double hb = hostBpm.load();
    if (getParam ("bpmHost") > 0.5f && hb > 1.0) return hb;
    return getParam ("bpm");
}

void MayDrummerProcessor::effectiveMeter (int& num, int& den) const
{
    int hn = hostNum.load(), hd = hostDen.load();
    if (getParam ("meterHost") > 0.5f && hn > 0 && hd > 0)
    {
        if (hd == 2) { hn *= 2; hd = 4; }
        else if (hd == 16) { if (hn % 2 == 0) { hn /= 2; hd = 8; } else { hn = 4; hd = 4; } }
        else if (hd != 4 && hd != 8) { hn = 4; hd = 4; }
        num = juce::jlimit (1, 16, hn);
        den = hd;
        return;
    }
    const auto& m = meters[juce::jlimit (0, (int) std::size (meters) - 1, (int) getParam ("meter"))];
    num = m.num; den = m.den;
}

GenSettings MayDrummerProcessor::makeSettings() const
{
    GenSettings s;
    effectiveMeter (s.meterNum, s.meterDen);
    const int barsChoice[] = { 1, 2, 4, 8 };
    s.bars = barsChoice[juce::jlimit (0, 3, (int) getParam ("bars"))];
    s.feelMode = (int) getParam ("feelMode");
    s.swing = getParam ("swing") / 100.0f;
    s.swing16 = getParam ("swing16") > 0.5f;
    s.timeFeel = (int) getParam ("timeFeel");

    // May Drummer: Y = forza (velocity), X = da dritto a imprevedibile
    const float x = getParam ("mdX"), y = getParam ("mdY");
    s.intensity = y;
    s.complexity = x * 0.9f;
    s.humTiming = juce::jlimit (0.0f, 1.0f, getParam ("humTiming") / 100.0f + 0.35f * x);
    s.humVelocity = juce::jlimit (0.0f, 1.0f, getParam ("humVel") / 100.0f + 0.45f * x);

    s.volume = getParam ("volume") / 100.0f;
    s.feel = getParam ("feel") / 100.0f;
    s.ghost = getParam ("ghost") / 100.0f;
    s.hatAccent = getParam ("hatAccent") / 100.0f;
    s.cymbalMode = (int) getParam ("cymbals");
    s.level = (int) getParam ("level");
    s.sideStick = (int) getParam ("sideStick");
    s.crashOnOne = getParam ("crash") > 0.5f;
    s.seed = (juce::uint32) getParam ("seed");
    for (int i = 0; i < NumGroups; ++i) s.mute[(size_t) i] = getParam ("mute" + juce::String (i)) > 0.5f;

    s.songBars = (int) getParam ("songBars");
    const int sec[] = { 4, 8, 16 };
    s.sectionBars = sec[juce::jlimit (0, 2, (int) getParam ("sectionBars"))];
    const int fe[] = { 0, 2, 4, 8 };
    s.fillEvery = fe[juce::jlimit (0, 3, (int) getParam ("fillEvery"))];
    s.ending = (int) getParam ("ending");
    s.countIn = getParam ("countIn") > 0.5f;
    s.altRiff = getParam ("altRiff") > 0.5f;
    return s;
}

void MayDrummerProcessor::regenerateNow()
{
    const auto& pat = currentPattern();
    const auto s = makeSettings();

    for (int i = 0; i < NumParts; ++i)
        partSeqs[(size_t) i] = DrumGenerator::renderPart (pat, i, s);
    songSeq = DrumGenerator::renderSong (pat, s);

    const bool song = getParam ("playMode") > 0.5f;
    viewSeq = song ? songSeq : partSeqs[(size_t) juce::jlimit (0, NumParts - 1, (int) getParam ("part"))];

    DrumSequence copy = viewSeq;
    {
        const juce::SpinLock::ScopedLockType sl (seqLock);
        std::swap (playSeq, copy);
    }
    generation++;
}

void MayDrummerProcessor::parameterChanged (const juce::String& id, float)
{
    if (id == "preview" || id == "followHost" || id == "channel" || id == "kitOn" || id == "kitVol") return;
    triggerAsyncUpdate();
}

void MayDrummerProcessor::handleAsyncUpdate()
{
    const int pp = pendingPart.exchange (-1);
    if (pp >= 0) { setParam ("part", (float) pp); }
    // cambiando preset si torna al ritmo originale
    const auto key = favKey ((int) getParam ("genre"), (int) getParam ("sub"), (int) getParam ("preset"));
    if (key != lastPatternKey)
    {
        if (lastPatternKey.isNotEmpty() && getParam ("alt") > 0.5f) setParam ("alt", 0.0f);
        lastPatternKey = key;
    }
    regenerateNow();
}

void MayDrummerProcessor::timerCallback()
{
    // Misura e BPM importati dal progetto della DAW
    const int hn = hostNum.load(), hd = hostDen.load();
    const double hb = hostBpm.load();
    bool changed = false;
    if ((hn != lastHostNum || hd != lastHostDen) && getParam ("meterHost") > 0.5f) changed = true;
    if (std::abs (hb - lastHostBpm) > 0.01 && getParam ("bpmHost") > 0.5f) generation++;   // solo la grafica
    lastHostNum = hn; lastHostDen = hd; lastHostBpm = hb;
    if (changed) triggerAsyncUpdate();
    if (pendingPart.load() >= 0) triggerAsyncUpdate();
}

void MayDrummerProcessor::randomise (bool alsoGenre)
{
    juce::Random r;
    if (alsoGenre) setParam ("genre", (float) r.nextInt (NumStyles));
    setParam ("sub", (float) r.nextInt (PatternLibrary::subsPerStyle));
    setParam ("preset", (float) r.nextInt (PatternLibrary::presetsPerSub));
    setParam ("seed", (float) (1 + r.nextInt (9998)));
    setParam ("alt", 0.0f);
}

void MayDrummerProcessor::prepareToPlay (double sr, int)
{
    sampleRate = sr;
    numPendingOffs = 0;
    synth.prepare (sr);
}

void MayDrummerProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const int numSamples = buffer.getNumSamples();
    const int channel = juce::jlimit (1, 16, (int) getParam ("channel"));
    if (mapChanged.exchange (false))
    {
        for (int i = 0; i < numPendingOffs; ++i)
            midi.addEvent (juce::MidiMessage::noteOff (pendingOffs[(size_t) i].channel, pendingOffs[(size_t) i].note), 0);
        numPendingOffs = 0;
        midi.addEvent (juce::MidiMessage::allNotesOff (channel), 0);
    }

    juce::MidiBuffer out;

    // Keyswitch: note 0..7 (C-2..G-2) scelgono la parte
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOnOrOff() && msg.getNoteNumber() < NumParts)
        {
            if (msg.isNoteOn()) { pendingPart = msg.getNoteNumber(); }
            continue;
        }
        out.addEvent (msg, meta.samplePosition);
        if (msg.isNoteOn() && getParam ("kitOn") > 0.5f)
            for (int p = 0; p < NumPieces; ++p)
                if (noteMap[(size_t) p].load() == msg.getNoteNumber()) { synth.trigger (p, msg.getVelocity(), meta.samplePosition); break; }
    }
    const bool kitOn = getParam ("kitOn") > 0.5f;
    const float kitGain = getParam ("kitVol") / 100.0f * 1.4f;

    bool hostPlaying = false;
    double ppqStart = 0.0;
    double bpm = 120.0;

    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) { bpm = *b; hostBpm = *b; }
            if (auto ts = pos->getTimeSignature()) { hostNum = ts->numerator; hostDen = ts->denominator; }
            hostPlaying = pos->getIsPlaying();
            if (auto q = pos->getPpqPosition()) ppqStart = *q;
        }
    }

    const bool follow = getParam ("followHost") > 0.5f;
    const bool preview = getParam ("preview") > 0.5f;
    bool playing = false;

    if (hostPlaying && follow) playing = true;
    else if (preview)
    {
        playing = true;
        bpm = effectiveBpm();
        ppqStart = internalPPQ;
    }

    if (! playing)
    {
        if (wasPlaying) out.addEvent (juce::MidiMessage::allNotesOff (channel), 0);
        wasPlaying = false;
        internalPPQ = 0.0;
        playPosition = -1.0;
        for (int i = 0; i < numPendingOffs; ++i)
            out.addEvent (juce::MidiMessage::noteOff (pendingOffs[(size_t) i].channel, pendingOffs[(size_t) i].note), 0);
        numPendingOffs = 0;
        if (kitOn) synth.render (buffer, kitGain);
        midi.swapWith (out);
        return;
    }
    wasPlaying = true;

    const double samplesPerPPQ = sampleRate * 60.0 / juce::jmax (20.0, bpm);
    const double ppqEnd = ppqStart + numSamples / samplesPerPPQ;
    if (! (hostPlaying && follow)) internalPPQ = ppqEnd;

    const juce::SpinLock::ScopedTryLockType sl (seqLock);
    if (sl.isLocked() && playSeq.lengthPPQ > 0.0)
    {
        const double len = playSeq.lengthPPQ;
        for (auto& e : playSeq.events)
        {
            double t = e.ppq + e.ms * bpm / 60000.0;
            t = std::fmod (t, len);
            if (t < 0) t += len;
            const double k = std::ceil ((ppqStart - t) / len - 1e-12);
            const double occ = t + k * len;
            if (occ >= ppqStart && occ < ppqEnd)
            {
                const int offset = juce::jlimit (0, numSamples - 1, (int) ((occ - ppqStart) * samplesPerPPQ));
                const int note = noteMap[(size_t) e.piece].load();
                out.addEvent (juce::MidiMessage::noteOn (channel, note, (juce::uint8) e.vel), offset);
                if (kitOn) synth.trigger (e.piece, e.vel, offset);
                if (numPendingOffs < (int) pendingOffs.size())
                    pendingOffs[(size_t) numPendingOffs++] = { note, channel, offset + (int) (samplesPerPPQ / 8.0) };
            }
        }
        double rel = std::fmod (ppqStart, len);
        if (rel < 0) rel += len;
        playPosition = rel / len;
    }

    if (kitOn) synth.render (buffer, kitGain);

    // Note-off in sospeso (anche quelli appena creati)
    for (int i = 0; i < numPendingOffs;)
    {
        auto& po = pendingOffs[(size_t) i];
        if (po.samplesLeft < numSamples)
        {
            out.addEvent (juce::MidiMessage::noteOff (po.channel, po.note), juce::jmax (0, po.samplesLeft));
            pendingOffs[(size_t) i] = pendingOffs[(size_t) --numPendingOffs];
        }
        else { po.samplesLeft -= numSamples; ++i; }
    }

    midi.swapWith (out);
}

// ---------------------------------------------------------------- File MIDI per il drag & drop
juce::String MayDrummerProcessor::baseFileName() const
{
    const auto& p = currentPattern();
    auto name = juce::String (PatternLibrary::styleName (p.style)) + "_"
              + PatternLibrary::get().subNames (p.style)[p.sub] + "_" + juce::String (p.name.substring (0, 2));
    const int alt = (int) getParam ("alt");
    if (alt > 0) name << "_Alt" << alt;
    return "MayDrummer_" + name.replaceCharacters (" /&", "__e").removeCharacters ("\\:*?\"<>|");
}

juce::File MayDrummerProcessor::makeTempFile (const juce::String& suffix) const
{
    // Una cartella nuova per ogni trascinamento: il nome del clip resta pulito,
    // ma la DAW non puo riusare un file importato in precedenza con le note vecchie.
    auto root = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("MayStudioMidiDrummer");
    auto dir = root.getChildFile (juce::String (juce::Time::currentTimeMillis()) + "_" + juce::String (exportCounter++));
    dir.createDirectory();
    return dir.getChildFile (baseFileName() + "_" + suffix.replaceCharacter (' ', '_') + ".mid");
}

juce::File MayDrummerProcessor::writeSequence (const DrumSequence& seq, const juce::String& suffix)
{
    std::array<int, NumPieces> map {};
    for (int i = 0; i < NumPieces; ++i) map[(size_t) i] = noteMap[(size_t) i].load();
    auto mf = DrumGenerator::toMidiFile (seq, map, (int) getParam ("channel"), effectiveBpm(),
                                         seq.meterNum, seq.meterDen, baseFileName() + " " + suffix);
    auto f = makeTempFile (suffix);
    f.deleteFile();
    juce::FileOutputStream os (f);
    if (os.openedOk()) { mf.writeTo (os, 1); os.flush(); }
    return f;
}

juce::File MayDrummerProcessor::writePartFile (int part)
{
    part = juce::jlimit (0, NumParts - 1, part);
    return writeSequence (partSeqs[(size_t) part], partName (part));
}

juce::File MayDrummerProcessor::writeSongFile()
{
    return writeSequence (songSeq, "Song");
}

juce::StringArray MayDrummerProcessor::writeInstrumentFiles (bool song, int group)
{
    const int part = juce::jlimit (0, NumParts - 1, (int) getParam ("part"));
    const auto& src = song ? songSeq : partSeqs[(size_t) part];
    const juce::String base = song ? juce::String ("Song") : juce::String (partName (part));
    juce::StringArray files;
    for (int g = 0; g < NumGroups; ++g)
    {
        if (group >= 0 && g != group) continue;
        DrumSequence one = src;
        one.events.clear();
        for (auto& e : src.events) if (groupOf (e.piece) == g) one.events.push_back (e);
        if (one.events.empty()) continue;
        files.add (writeSequence (one, base + " " + groupName (g)).getFullPathName());
    }
    return files;
}

juce::StringArray MayDrummerProcessor::writeAllPartFiles()
{
    juce::StringArray files;
    for (int i = 0; i < NumParts; ++i) files.add (writePartFile (i).getFullPathName());
    return files;
}

// ---------------------------------------------------------------- Stato
void MayDrummerProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    auto map = state.getOrCreateChildWithName ("NoteMap", nullptr);
    for (int i = 0; i < NumPieces; ++i) map.setProperty ("n" + juce::String (i), noteMap[(size_t) i].load(), nullptr);
    state.setProperty ("presetName", currentUserPreset, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void MayDrummerProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
    {
        auto state = juce::ValueTree::fromXml (*xml);
        if (state.hasType (apvts.state.getType()))
        {
            auto map = state.getChildWithName ("NoteMap");
            if (map.isValid())
                for (int i = 0; i < NumPieces; ++i)
                    noteMap[(size_t) i] = (int) map.getProperty ("n" + juce::String (i), gmNote (i));
            currentUserPreset = state.getProperty ("presetName").toString();
            mapChanged = true;
            apvts.replaceState (state);
            triggerAsyncUpdate();
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MayDrummerProcessor(); }

juce::AudioProcessorEditor* MayDrummerProcessor::createEditor() { return new MayDrummerEditor (*this); }
