# May Studio Midi Drummer

Plug-in VST3 (e AU su Mac) che genera tracce MIDI di batteria e le porta nella DAW **trascinandole direttamente dall'interfaccia**.

## Flusso di lavoro (pagina DRUMMER)

1. **Genere**: 16 generi (Rock, Pop, Funk, Blues, Country, Jazz, Metal, Punk, Reggae, Hip Hop, R&B Soul, Disco, Latin, Ballad, Motown, Electronic).
2. **Sottogenere**: 4 per genere (es. Rock: Classic Rock, Arena Rock, Hard Rock, Rock 16).
3. **Misura e BPM**: 4/4, 3/4, 2/4, 5/4, 6/4, 7/4, 5/8, 6/8, 7/8, 9/8, 12/8. Con "Da progetto" vengono letti dalla DAW.
4. **Pattern preset**: 5 per sottogenere (Base, Ride/Open Hat/Comping, Plus, Combinato, Sync). In totale 320 pattern.
5. **Main / Riff / Finale**: ogni pattern ha Main, Riff, 3 Fill e 3 Finali. Durata predefinita 2 battute (1, 2, 4 o 8).
6. **May Drummer**: quadrato con un puntino trascinabile. In alto = forte, in basso = leggero; a sinistra = dritto, a destra = imprevedibile. Al rilascio mostra a destra l'anteprima di Main, Riff e Finale; ogni riga si trascina nella DAW.

Novità:
- **Frecce ▲▼**: semplificano o arricchiscono la traccia su 5 livelli per lato (da Essenziale a Virtuoso).
- **Rullante**: Normale, Side stick nel Main, Side stick sempre.
- **Ritmo alternativo**: ricombina cassa, piatti e percussioni con altri groove dello stesso genere; ORIGINALE per tornare indietro.
- **Browser**: tutti i 320 preset con filtro per genere, ricerca e preferiti ★ (salvati per tutti i progetti).
- **Singoli strumenti**: trascina solo Cassa, Rullante, Hi-Hat, Tom, Piatti o Percussioni, oppure SEPARATI per avere un file per strumento.

Controlli aggiuntivi: Humanize Timing e Velocity, Volume, Push/Laid back, Ghost notes, Accenti dei piatti, Swing (1/8 o 1/16), Feel Auto/Dritto/Shuffle, Half-time e Double-time, Hi-Hat/Ride forzati, Crash su 1, Casuale.

## Importare nella DAW

Trascina le zone dorate (o direttamente le anteprime) sulla traccia MIDI o sulla traccia dello strumento di batteria:
- **Trascina la parte**: la parte selezionata.
- **Tutte le parti**: 8 file (Main, Riff, Fill 1-3, Finale 1-3).
- **Singoli strumenti**: la riga sotto la vista (e nella pagina SONG) esporta un solo strumento o tutti separati.
- **Trascina la song** (pagina SONG): traccia completa con count-in, alternanza Main/Riff, fill ogni N battute e finale.

I file contengono tempo e misura; canale MIDI predefinito 10, mappa General MIDI (modificabile nella pagina MAPPA).

## Uso dal vivo

Il plug-in invia anche MIDI in tempo reale, sincronizzato con il trasporto della DAW. Keyswitch C-2..G-2 per cambiare parte (Main, Riff, Fill 1-3, Finale 1-3). Il pulsante ANTEPRIMA suona anche a trasporto fermo.

## Compilare

Serve CMake 3.22+ e un compilatore C++17 (Visual Studio 2022 su Windows, Xcode su Mac). JUCE viene scaricato in automatico.

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Il VST3 si trova in `build/MayStudioMidiDrummer_artefacts/Release/VST3/`.

**Senza installare nulla**: carica la cartella su un repository GitHub; il workflow in `.github/workflows/build.yml` compila automaticamente le versioni Windows e Mac (scheda *Actions*, poi *Artifacts*).

## Installare

- **Windows**: copia `May Studio Midi Drummer.vst3` in `C:\Program Files\Common Files\VST3`.
- **Mac**: copia il `.vst3` in `/Library/Audio/Plug-Ins/VST3` (e il `.component` in `/Library/Audio/Plug-Ins/Components` per Logic). Il plug-in non è firmato: la prima volta esegui `xattr -cr "/Library/Audio/Plug-Ins/VST3/May Studio Midi Drummer.vst3"` nel Terminale.
