#include "PatternLibrary.h"
#include <set>

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
            { "Classic Rock", 4, "K:x-------x-x-----|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Arena Rock", 4, "K:x---x---x---x---|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Hard Rock", 4, "K:x-----x-x-x-----|S:----X-------X--g|H:X-x-X-x-X-x-X-x-" },
            { "Rock 16", 4, "K:x-------x-x---x-|S:----X--g----X---|H:XoxoXoxoXoxoXoxo" },
            { "Rock Shuffle", 3, "K:x--x--x-----|S:---X-----X--|R:x-xx-xx-xx-x" },
            { "Tom Groove", 4, "K:x-------x-x-----|S:----X-------X---|3:x-x-x-x-x-x-x-x-" },
            { "Garage", 4, "K:x-x-----x-x-----|S:----X-------X---|C:x-x-x-x-x-x-x-x-" },
            { "Southern Rock", 4, "K:x-----x-x-----x-|S:----X-------X---|H:x-x-x-x-x-x-x-x-" },
            { "Alt Half-Time", 4, "K:x-----x-----x---|S:--------X-------|H:x-x-x-x-x-x-x-x-" },
            { "Rock 2 Bar", 4, "K:x-------x-x-----x-----x-x-x-----|S:----X-------X-------X-------X--g|H:X-x-X-x-X-x-X-x-X-x-X-x-X-x-X---|O:------------------------------x-" },
        },
        { // Pop
            { "Pop Drive", 4, "K:x-------x-------|S:----X-------X---|H:XoxoXoxoXoxoXoxo" },
            { "Pop Bounce", 4, "K:x--x--x---x-----|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Dance Pop", 4, "K:x-------x-x-----|S:----X-------X---|L:----x-------x---|H:X-x-X-x-X-x-X---|O:--------------x-" },
            { "Acoustic Pop", 4, "K:x-----x---x--x--|S:----X-------X---|M:xoxoxoxoxoxoxoxo|H:x---x---x---x---" },
            { "Pop Rock", 4, "K:x-----x-x-x-----|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Synth Pop", 4, "K:x---x---x---x---|S:----X-------X---|H:--x---x---x---x-|L:----x-------x---" },
            { "Pop Shuffle", 3, "K:x-----x--x--|S:---X-----X--|H:x-xx-xx-xx-x|A:---x-----x--" },
            { "Half-Time Pop", 4, "K:x-----x---x-----|S:--------X-------|H:x-x-x-x-x-x-x-x-|M:oooooooooooooooo" },
            { "Dembow Pop", 4, "K:x---x---x---x---|S:---x--x----x--x-|H:x-x-x-x-x-x-x-x-" },
            { "Pop 2 Bar", 4, "K:x-------x-x-----x--x----x-------|S:----X-------X-------X-------X---|H:x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-|L:----x-------x-------x-------x---" },
        },
        { // Funk
            { "Funk 16", 4, "K:x-x-------x--x--|S:----X--g-g--X--g|H:XxxxXxxxXxxxXxxx" },
            { "Break Funk", 4, "K:x-x-----x-x--x--|S:----X--g-g-gX--g|H:xxxxxxxxxxx-xxxx|O:-----------x----" },
            { "Syncopated Funk", 4, "K:x--x--x---x--x--|S:----X-g-----X-g-|H:XoXoXoXoXoXoXoXo" },
            { "Linear Funk", 4, "K:x------xx-x-----|S:----X--g-g--X---|H:x-x-x-x-x-x-x-x-|O:--------------x-" },
            { "Second Line", 4, "K:x-----x-x-----x-|S:X-gX-g-XgX-gX-g-|H:x---x---x---x---" },
            { "Funk Shuffle", 3, "K:x--x-----x-x|S:-g-X-g-g-Xg-|H:x-xx-xx-xx-x" },
            { "Go-Go", 4, "K:x--x--x---x-----|S:----X-------X---|Q:x-x--x-x-x-x--x-|H:x-x-x-x-x-x-x-x-" },
            { "P-Funk", 4, "K:x-x---x---x--x--|S:----X--g-g--X---|H:xxxxxxxxxxxxxxxx|L:----x-------x---" },
            { "Fusion", 4, "K:x--x---x--x---x-|S:----X-g--g--X-g-|R:x-x-x-x-x-x-x-x-|B:x---x---x---x---" },
            { "Funk 2 Bar", 4, "K:x-x-------x--x--x--x---x--x-----|S:----X--g-g--X--g-g--X--g----X-g-|H:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|O:----------------------x---------" },
        },
        { // Blues
            { "Chicago Shuffle", 3, "K:x-----x-----|S:---X-----X--|H:x-xx-xx-xx-x" },
            { "Texas Shuffle", 3, "K:x--x--x--x--|S:---X-----X--|R:x-xx-xx-xx-x" },
            { "Slow Blues 12-8", 3, "K:x-----x--x--|S:---X-----X--|H:XooXooXooXoo" },
            { "Shuffle Ghost", 3, "K:x----xx-----|S:-g-X-g-g-X-g|H:x-xx-xx-xx-x" },
            { "Jump Blues", 3, "K:x--x--x--x--|S:---X-----X--|R:X-xx-xX-xx-x" },
            { "Slow Blues Ride", 3, "K:x-----x-----|S:---X-----X--|R:xxxxxxxxxxxx|P:---x-----x--" },
            { "Blues Rock", 4, "K:x-----x-x-------|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Boogie", 3, "K:x--x--x--x--|S:--gX--g--X-g|H:x-xx-xx-xx-x" },
            { "Delta Train", 4, "K:x-------x-------|S:gxgxgxgXgxgxgxgX|P:----x-------x---" },
            { "Blues 2 Bar", 3, "K:x-----x-----x--x--x-----|S:---X-----X-----X-----X-g|H:x-xx-xx-xx-xx-xx-xx-xx-x" },
        },
        { // Country
            { "Train Beat", 4, "K:x-------x-------|S:gxgXgxgXgxgXgxgX|P:----x-------x---" },
            { "Two Step", 4, "K:x-------x-------|X:----x-------x---|H:x-x-x-x-x-x-x-x-" },
            { "Country Rock", 4, "K:x-----x-x-------|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Bluegrass", 4, "K:x-------x-------|S:----X-------X---|H:----x-------x---|M:x-x-x-x-x-x-x-x-" },
            { "Country Shuffle", 3, "K:x-----x-----|X:---x-----x--|H:x-xx-xx-xx-x" },
            { "Honky Tonk", 4, "K:x-------x-------|S:----X-------X---|R:x-x-x-x-x-x-x-x-" },
            { "Bakersfield", 4, "K:x-----x-x-----x-|S:----X-------X---|H:x-xxx-xxx-xxx-xx" },
            { "Country Pop", 4, "K:x-----x---x-----|S:----X-------X---|H:x-x-x-x-x-x-x-x-|A:----x-------x---" },
            { "Polka Two-Beat", 4, "K:x-------x-------|S:--x---x---x---x-|H:--x---x---x---x-" },
            { "Country 2 Bar", 4, "K:x-------x-x-----x-------x-----x-|S:----X-------X-------X-------X---|H:x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-" },
        },
        { // Jazz
            { "Medium Swing", 3, "R:X--x-xX--x-x|P:---x-----x--|K:g--g--g--g--|S:--------g---" },
            { "Ballad Swing", 3, "R:x--x-xx--x-x|P:---x-----x--|S:g-g--g---g-g|K:g-----g-----" },
            { "Bebop", 3, "R:x--x-xx--x-x|P:---x-----x--|S:-----g--g---|K:-----------o" },
            { "Two Feel", 3, "R:x--x--x--x--|P:---x-----x--|K:o-----o-----|S:--------g--g" },
            { "Up Tempo", 3, "R:x--x-xx--x-x|P:---x-----x--|S:--g-----g---|K:g-----g-----" },
            { "Brushes", 3, "S:oooXoooooXoo|P:---x-----x--|K:g-----g-----" },
            { "Afro 6-8", 3, "B:x-x-xx-x-x-x|K:x-----x-----|S:--g--x--g--x|P:---x-----x--" },
            { "Jazz Shuffle", 3, "R:x-xx-xx-xx-x|S:---X-----X--|K:x--g--x--g--|P:---x-----x--" },
            { "Latin Jazz", 4, "K:x--xx--xx--xx--x|X:--x--x---x--x---|R:x-x-x-x-x-x-x-x-" },
            { "Jazz 2 Bar", 3, "R:x--x-xx--x-xx--x-xx--x-x|P:---x-----x-----x-----x--|S:--------g-----g-----g--g|K:g-----g-----g-----g----o" },
        },
        { // Metal
            { "Double Kick", 4, "K:xxxxxxxxxxxxxxxx|S:----X-------X---|C:x---x---x---x---" },
            { "Thrash", 4, "K:x---x---x---x---|S:--X---X---X---X-|R:X-x-X-x-X-x-X-x-" },
            { "Groove Metal", 4, "K:xx------xx-x----|S:--------X-------|H:X-x-X-x-X-x-X-x-" },
            { "Blast Beat", 4, "K:x-x-x-x-x-x-x-x-|S:-x-x-x-x-x-x-x-x|R:x-x-x-x-x-x-x-x-" },
            { "Gallop", 4, "K:x-xxx-xxx-xxx-xx|S:----X-------X---|C:x---x---x---x---" },
            { "Djent", 4, "K:xx-x--x-xx-x-x--|S:----X-------X---|Y:x-------x-------" },
            { "Doom", 4, "K:x-------x-x-----|S:--------X-------|R:x---x---x---x---" },
            { "Breakdown", 4, "K:x--x--x---x--x--|S:--------X-------|Y:x---x---x---x---" },
            { "Power Metal", 4, "K:xxxxxxxxxxxxxxxx|S:----X-------X---|R:x-x-x-x-x-x-x-x-" },
            { "Metal 2 Bar", 4, "K:x-x-x-x-x-x-x-x-xxxxxxxxxxxxxxxx|S:----X-------X-------X-------X---|C:x---x---x---x---x---x---x---x---" },
        },
        { // Punk
            { "Punk Rock", 4, "K:x-x---x-x-x---x-|S:----X-------X---|H:X-x-X-x-X-x-X-x-" },
            { "Skate Punk", 4, "K:x---x---x---x---|S:--X---X---X---X-|H:x-x-x-x-x-x-x-x-" },
            { "D-Beat", 4, "K:x--x--x-x--x----|S:----X-------X---|C:x-x-x-x-x-x-x-x-" },
            { "Pop Punk", 4, "K:x-x-x-x-x-x-x-x-|S:----X-------X---|R:X-x-X-x-X-x-X-x-" },
            { "Hardcore", 4, "K:x-x-x-x-x-x-x-x-|S:--X---X---X---X-|C:x---x---x---x---" },
            { "Ska Punk", 4, "K:x-------x-------|S:----X-------X---|H:--x---x---x---x-" },
            { "Oi", 4, "K:x---x-x-x---x-x-|S:----X-------X---|H:x-x-x-x-x-x-x-x-" },
            { "Punk Half-Time", 4, "K:x-x---x-x-x-----|S:--------X-------|C:x-x-x-x-x-x-x-x-" },
            { "Surf Punk", 4, "K:x-------x-------|S:----X-------X---|3:x-xxx-xxx-xxx-xx" },
            { "Punk 2 Bar", 4, "K:x-x---x-x-x---x-x-x---x-x-xx-xx-|S:----X-------X-------X-------X-X-|H:x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-" },
        },
        { // Reggae
            { "One Drop", 4, "K:--------x-------|X:--------X-------|H:x-x-x-x-x-x-x-x-" },
            { "Rockers", 4, "K:x-------x-------|S:----X-------X---|H:-xx--xx--xx--xx-" },
            { "Steppers", 4, "K:x---x---x---x---|X:----x-------x---|H:--x---x---x---x-" },
            { "Roots", 4, "K:--------x-------|S:--------X-------|H:x-x-x-x-x-x-x---|O:--------------x-" },
            { "Ska", 4, "K:x-------x-------|S:----X-------X---|H:--x---x---x---x-" },
            { "Rocksteady", 4, "K:--------x-------|X:--------x-------|H:x-xxx-xxx-xxx-xx" },
            { "Dub", 4, "K:--------x-------|S:--------X-------|H:--x---x---x---x-|3:--------------x-" },
            { "Dancehall", 4, "K:x-----x-x-----x-|X:---x--x----x--x-|L:----x-------x---|H:x-x-x-x-x-x-x-x-" },
            { "Reggae Shuffle", 3, "K:------x-----|X:------x-----|H:x-xx-xx-xx-x" },
            { "Reggae 2 Bar", 4, "K:--------x---------------x-----x-|X:--------x---------------x-------|H:x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-" },
        },
        { // Hip Hop
            { "Boom Bap", 4, "K:x------x-x------|S:----X-------X---|H:x-x-x-x-x-x-x-x-" },
            { "Bap 16", 4, "K:x-x----x--x-----|S:----X-------X--g|H:XoxoXoxoXoxoXoxo" },
            { "Trap", 4, "K:x-----x---x-----|S:--------X-------|H:xxxxxxxxxxxxxxxx" },
            { "Lo-Fi", 4, "K:x-----x---x-x---|S:----X-------X---|L:----x-------x---|H:x-x-x-x-x-x-x-x-" },
            { "Old School", 4, "K:x--x--x-x-------|S:----X-------X---|H:x-x-x-x-x-x-x-x-|L:----x-------x---" },
            { "G-Funk", 4, "K:x-----x-x-x-----|S:----X-------X---|H:xxxxxxxxxxxxxxxx" },
            { "Drill", 4, "K:x-----x---x--x--|S:--------X-----X-|H:x-xxx-x-x-xxx-x-" },
            { "Lo-Fi Shuffle", 3, "K:x-----x-x---|S:---X-----X--|H:x-xx-xx-xx-x" },
            { "Amen Break", 4, "K:x-x-------xx----|S:----X--g-gX-X--g|R:x-x-x-x-x-x-x-x-" },
            { "Hip Hop 2 Bar", 4, "K:x------x-x------x-x----x--x-----|S:----X-------X-------X-------X--g|H:x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-" },
        },
        { // R&B Soul
            { "Soul", 4, "K:x-----x-x-------|S:----X--g----X---|H:x-x-x-x-x-x-x-x-" },
            { "Neo Soul", 4, "K:x--x------x--x--|S:----X---g---X-g-|H:oooooooooooooooo" },
            { "Classic R&B", 4, "K:x-------x-x-----|X:----x-------x---|H:x-x-x-x-x-x-x-x-" },
            { "Ghost Groove", 4, "K:x-------x-x-----|S:-g-gX-g--g-gX-g-|H:x-x-x-x-x-x-x-x-" },
            { "Philly Soul", 4, "K:x---x---x---x---|S:----X-------X---|H:xxxxxxxxxxxxxxxx|O:--x---x---x---x-" },
            { "Gospel Shuffle", 3, "K:x--x--x--x-x|S:-g-X-gg-gX-g|H:x-xx-xx-xx-x" },
            { "Slow Jam", 4, "K:x------x--x-----|S:--------X-------|H:x-x-x-x-x-x-x-x-" },
            { "Soul Shuffle", 3, "K:x-----x--x--|X:---x-----x--|R:x-xx-xx-xx-x" },
            { "Southern Soul", 4, "K:x-----x-x-----x-|S:----X-------X---|H:x-x-x-x-x-x-x-x-|A:----x-------x---" },
            { "Soul 2 Bar", 4, "K:x-----x-x-------x-----x---x-----|S:----X--g----X-------X--g----X-g-|H:x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-" },
        },
        { // Disco
            { "Disco", 4, "K:x---x---x---x---|S:----X-------X---|H:x---x---x---x---|O:--x---x---x---x-" },
            { "Disco 16", 4, "K:x---x---x---x---|S:----X-------X---|H:xxxxxx-xxxxxxx-x|O:------x-------x-" },
            { "Nu Disco", 4, "K:x---x---x---x---|S:----X-------X---|L:----x-------x---|O:--x---x---x---x-|A:xxxxxxxxxxxxxxxx" },
            { "Cowbell Disco", 4, "K:x---x---x---x---|S:----X--g----X---|H:x-x-x-x-x-x-x-x-|Q:x--x--x---x--x--" },
            { "Philly Disco", 4, "K:x---x---x---x---|S:----X-------X---|H:xxxxxxxxxxxxxxxx|O:--x---x---x---x-|A:----x-------x---" },
            { "Hustle", 4, "K:x---x---x---x---|S:----X--x----X---|H:x-x-x-x-x-x-x-x-|Q:--x---x---x---x-" },
            { "Funk Disco", 4, "K:x---x---x-x-x---|S:----X--g----X--g|H:xxxxxxxxxxxxxxxx" },
            { "Italo Disco", 4, "K:x---x---x---x---|L:----x-------x---|H:--x---x---x---x-|M:xxxxxxxxxxxxxxxx" },
            { "Disco Tom", 4, "K:x---x---x---x---|S:----X-------X---|O:--x---x---x---x-|1:--------------x-|3:---------------x" },
            { "Disco 2 Bar", 4, "K:x---x---x---x---x---x---x---x---|S:----X-------X-------X-------X--g|O:--x---x---x---x---x---x---x---x-|H:x---x---x---x---x---x---x---x---" },
        },
        { // Latin
            { "Bossa Nova", 4, "K:x--xx--xx--xx--x|X:x--x--x---x--x--|H:x-x-x-x-x-x-x-x-" },
            { "Samba", 4, "K:o--Xo--Xo--Xo--X|X:x-x--x-x-x-x--x-|H:XxxxXxxxXxxxXxxx" },
            { "Cha Cha", 4, "K:x-------x-------|Q:X---x---X---x---|X:------x-------x-|3:------------x-x-" },
            { "Songo", 4, "K:---x------x---x-|S:--g---x-g---x---|Q:X-x-X-x-x-x-X-x-|3:------x-------x-" },
            { "Mambo", 4, "K:---x------x-----|Q:X-x-x-x-X-x-x-x-|X:----x--x--x-----|3:------xx------xx" },
            { "Salsa", 4, "K:------x-------x-|B:x-x-xx-x-x-xx-x-|Q:x---x---x---x---" },
            { "Merengue", 4, "K:x---x---x---x---|S:--x---x---x-xxx-|H:x-x-x-x-x-x-x-x-" },
            { "Cumbia", 4, "K:x-------x-------|X:----x-------x---|M:x-xxx-xxx-xxx-xx" },
            { "Reggaeton", 4, "K:x---x---x---x---|S:---x--x----x--x-|H:x-x-x-x-x-x-x-x-" },
            { "Bossa 2 Bar", 4, "K:x--xx--xx--xx--xx--xx--xx--xx--x|X:x--x--x---x-------x--x---x--x---|H:x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-" },
        },
        { // Ballad
            { "Ballad", 4, "K:x-------x-------|S:----x-------x---|H:o-o-o-o-o-o-o-o-" },
            { "Rim Ballad", 4, "K:x-----x-x-------|X:----x-------x---|H:o-o-o-o-o-o-o-o-" },
            { "Power Ballad", 4, "K:x-------x-x-----|S:----X-------X---|R:x-x-x-x-x-x-x-x-" },
            { "Half Time Ballad", 4, "K:x-----x---------|S:--------X-------|H:o-o-o-o-o-o-o-o-" },
            { "Ballad 12-8", 3, "K:x-----x-----|S:---X-----X--|H:oooooooooooo" },
            { "Brush Ballad", 3, "S:oooXoooooXoo|K:g-----g-----|P:---x-----x--" },
            { "Piano Ballad", 4, "K:x-------x-------|X:--------x-------|R:o-o-o-o-o-o-o-o-" },
            { "Pop Ballad 16", 4, "K:x------xx-------|S:----X-------X---|H:oooooooooooooooo" },
            { "Soul Ballad", 4, "K:x-----x-x-------|S:----x--g----x---|H:o-o-o-o-o-o-o-o-|A:----o-------o---" },
            { "Ballad 2 Bar", 4, "K:x-------x-x-----x-------x-------|S:----x-------x-------x-------x--g|H:o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-" },
        },
        { // Motown
            { "Motown", 4, "K:x-----x-x-----x-|S:x---X---x---X---|H:x-x-x-x-x-x-x-x-|A:----x-------x---" },
            { "Northern Soul", 4, "K:x-x---x-x-x---x-|S:----X-------X---|A:----x-------x---|H:x-x-x-x-x-x-x-x-" },
            { "Tamb 16", 4, "K:x---x---x---x---|S:----X-------X---|A:xxxxxxxxxxxxxxxx" },
            { "Soul Stomp", 4, "K:x---x---x---x---|S:----X-------X---|L:----x-------x---|H:x-x-x-x-x-x-x-x-" },
            { "Motown Shuffle", 3, "K:x-----x-----|S:x--X--x--X--|H:x-xx-xx-xx-x|A:---x-----x--" },
            { "Stax", 4, "K:x-----x-x-------|S:----X-------X---|H:x-x-x-x-x-x-x-x-|A:----x-------x---" },
            { "Girl Group", 4, "K:x---x---x---x---|S:----X-------X---|3:x---x---x---x---|A:----x-------x---" },
            { "Detroit", 4, "K:x-----x---x--x--|S:----X-------X---|H:xxxxxxxxxxxxxxxx|A:----x-------x---" },
            { "Uptown", 4, "K:x---x-x-x---x-x-|S:----X-------X---|H:x-x-x-x-x-x-x-x-" },
            { "Motown 2 Bar", 4, "K:x-----x-x-----x-x-x---x-x-----x-|S:x---X---x---X---x---X---x---X---|A:----x-------x-------x-------x---|H:x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-x-" },
        },
        { // Electronic
            { "House", 4, "K:x---x---x---x---|L:----x-------x---|H:x-x-x-x-x-x-x-x-|O:--x---x---x---x-" },
            { "Techno", 4, "K:x---x---x---x---|S:----o-------o---|H:--x---x---x---x-|R:x-x-x-x-x-x-x-x-" },
            { "Breakbeat", 4, "K:x-x-------x-----|S:----X--g-g--X---|H:x-x-x-x-x-x-x-x-" },
            { "Drum & Bass", 4, "K:x---------x-----|S:----X-------X---|H:x-x-x-x-x-x-x-x-" },
            { "Trance", 4, "K:x---x---x---x---|O:--x---x---x---x-|L:----x-------x---|H:xxxxxxxxxxxxxxxx" },
            { "Dubstep", 4, "K:x---------x-----|S:--------X-------|H:x-x-x-x-x-x-x-x-" },
            { "UK Garage", 4, "K:x-----x---x-----|S:----X-------X---|H:--x---x---x---x-|M:x-xx-xx-x-xx-xx-" },
            { "Trap EDM", 4, "K:x-------x--x----|S:--------X-------|H:xxxxxxxxxxxxxxxx|L:--------x-------" },
            { "Synthwave", 4, "K:x---x---x---x---|S:----X-------X---|H:x-x-x-x-x-x-x-x-" },
            { "EDM 2 Bar", 4, "K:x---x---x---x---x---x---x---x---|L:----x-------x-------x-------x---|O:--x---x---x---x---x---x---x---x-|S:------------------------xxxxxxxx" },
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

void applyVariantLight (Pattern& p)
{
    const int L = p.stepsPerBar * p.coreBars;
    auto& sn = p.lines[Snare];
    for (auto& v : sn) if (v > 0 && v < 55) v = 0;
    for (int h : { (int) HHClosed, (int) Ride, (int) Shaker, (int) Tambourine })
    {
        auto& l = p.lines[(size_t) h];
        for (int i = 0; i < (int) l.size() && i < L; ++i)
        {
            const int k = i % p.stepsPerBar;
            if ((p.grid == 4 && k % 2 == 1) || (p.grid == 3 && k % 3 == 1)) l[(size_t) i] = 0;
        }
    }
    if (p.grid == 4)
        for (size_t i = 1; i < p.lines[Kick].size(); i += 2) p.lines[Kick][i] = 0;
    for (int h : { (int) HHClosed, (int) Ride, (int) HHOpen })
        for (auto& v : p.lines[(size_t) h]) if (v > 78) v = 78;
}

void applyVariantGhost (Pattern& p)
{
    const std::vector<int> pos = p.grid == 4 ? std::vector<int> { 3, 6, 9, 11, 14 } : std::vector<int> { 1, 4, 7, 10 };
    for (int st : pos)
    {
        const bool near = (st > 0 && stepUsed (p, Snare, st - 1) && p.lines[Snare][(size_t) st - 1] >= 90)
                       || (st + 1 < p.stepsPerBar && stepUsed (p, Snare, st + 1) && p.lines[Snare][(size_t) st + 1] >= 90);
        if (! near && ! stepUsed (p, Kick, st)) setIfEmpty (p, Snare, st, 40);
    }
}

void applyVariantPerc (Pattern& p)
{
    const int s = p.style;
    const bool g4 = p.grid == 4;
    std::vector<int> e8;
    for (int i = 0; i < p.stepsPerBar; i += (g4 ? 2 : (i % 3 == 0 ? 2 : 1))) e8.push_back (i);

    if (s == SLatin)
    {
        for (int i = 0; i < p.stepsPerBar; i += p.grid) setIfEmpty (p, Cowbell, i, 95);
        for (int i : e8) setIfEmpty (p, Shaker, i, 70);
    }
    else if (s == SFunk || s == SSoul || s == SMotown || s == SDisco || s == SPop || s == SHipHop)
    {
        setIfEmpty (p, Tambourine, p.grid, 95);
        setIfEmpty (p, Tambourine, p.grid * 3, 95);
        for (int i = 0; i < p.stepsPerBar; ++i) setIfEmpty (p, Shaker, i, i % 2 ? 55 : 72);
    }
    else if (s == SJazz)
        for (int i = 0; i < p.stepsPerBar; ++i) setIfEmpty (p, Shaker, i, 50);
    else if (s == SReggae)
        for (int i = p.grid / 2; i < p.stepsPerBar; i += p.grid) setIfEmpty (p, Cowbell, i, 80);
    else if (s == SElectronic)
    {
        for (int i = 0; i < p.stepsPerBar; ++i) setIfEmpty (p, Shaker, i, i % 2 ? 55 : 72);
        setIfEmpty (p, Clap, p.grid, 100);
        setIfEmpty (p, Clap, p.grid * 3, 100);
    }
    else
        for (int i : e8) setIfEmpty (p, Tambourine, i, i % p.grid == 0 ? 92 : 72);
}

void applyVariantPush (Pattern& p)
{
    const bool g4 = p.grid == 4;
    const int st = g4 ? 14 : 11;
    if (! stepUsed (p, Snare, st)) setIfEmpty (p, Kick, st, 95);
    if (p.uses (HHClosed))
    {
        auto& l = p.lines[HHClosed];
        for (int b = 0; b < p.coreBars; ++b) l[(size_t) (b * p.stepsPerBar + st)] = 0;
        setIfEmpty (p, HHOpen, st, 100);
    }
    else setIfEmpty (p, RideBell, st, 100);
    if (g4) setIfEmpty (p, Snare, 15, 70);
}

void scaleVel (Pattern& p)
{
    for (auto& l : p.lines) for (auto& v : l) if (v > 0) v = (juce::uint8) juce::jmax (30, juce::roundToInt (v * 0.78f));
}
void forceGhost (Pattern& p)
{
    const std::vector<int> pos = p.grid == 4 ? std::vector<int> { 1, 5, 10, 13 } : std::vector<int> { 2, 5, 8, 11 };
    for (int st : pos) if (! stepUsed (p, Snare, st) && ! stepUsed (p, Kick, st)) setIfEmpty (p, Snare, st, 40);
}
void altKick (Pattern& p)
{
    const std::vector<int> pos = p.grid == 4 ? std::vector<int> { 7, 11 } : std::vector<int> { 2, 8 };
    for (int st : pos) if (! stepUsed (p, Snare, st) && ! stepUsed (p, SideStick, st)) setIfEmpty (p, Kick, st, 82);
}
void accentCym (Pattern& p)
{
    setIfEmpty (p, China, 0, 110);
    setIfEmpty (p, Crash, p.stepsPerBar / 2, 100);
}

juce::String linesKey (const Pattern& p)
{
    juce::String k;
    for (auto& l : p.lines)
    {
        bool any = false;
        for (auto v : l) if (v) { any = true; break; }
        if (! any) { k << "|"; continue; }
        for (auto v : l) k << juce::String::toHexString ((int) v) << ",";
        k << "|";
    }
    return k;
}
} // namespace

namespace
{
// ---------------- 15 nuovi elementi per le combinazioni (preset 11-100)
int hpOf (const Pattern& p) { return p.uses (HHClosed) ? (int) HHClosed : (p.uses (Ride) ? (int) Ride : -1); }
std::vector<juce::uint8>& lineOf (Pattern& p, int piece)
{
    auto& l = p.lines[(size_t) piece];
    if (l.empty()) l.assign ((size_t) (p.stepsPerBar * p.coreBars), 0);
    return l;
}

void opFour (Pattern& p) { for (int i = 0; i < p.stepsPerBar; i += p.grid) setIfEmpty (p, Kick, i, 100); }
void opKick (Pattern& p, int k)
{
    static const std::vector<std::vector<int>> g4 { { 3, 10 }, { 6, 11 }, { 2, 9, 14 } }, g3 { { 2, 8 }, { 5, 11 }, { 1, 7 } };
    for (int st : (p.grid == 4 ? g4 : g3)[(size_t) k])
        if (! stepUsed (p, Snare, st) && ! stepUsed (p, SideStick, st)) setIfEmpty (p, Kick, st, 84);
}
void opKickA (Pattern& p) { opKick (p, 0); }
void opKickB (Pattern& p) { opKick (p, 1); }
void opKickC (Pattern& p) { opKick (p, 2); }
void opUpbeat (Pattern& p)
{
    const int h = hpOf (p);
    if (h < 0) return;
    auto& l = p.lines[(size_t) h];
    for (size_t i = 0; i < l.size(); ++i)
    {
        if (! l[i]) continue;
        const int k = (int) i % p.stepsPerBar;
        const bool off = p.grid == 4 ? k % 4 == 2 : k % 3 == 2;
        const bool on = k % p.grid == 0;
        l[i] = off ? 112 : (on ? 70 : l[i]);
    }
}
void opOpen24 (Pattern& p)
{
    const std::vector<int> pos = p.grid == 4 ? std::vector<int> { 6, 14 } : std::vector<int> { 5, 11 };
    for (int st : pos)
    {
        if (p.uses (HHClosed))
        {
            auto& l = p.lines[HHClosed];
            for (int b = 0; b < p.coreBars; ++b) l[(size_t) (b * p.stepsPerBar + st)] = 0;
            setIfEmpty (p, HHOpen, st, 96);
        }
        else setIfEmpty (p, RideBell, st, 96);
    }
}
void opTom (Pattern& p)
{
    if (p.uses (HHClosed)) { moveLine (p, HHClosed, TomFloor, 80); moveLine (p, HHOpen, TomFloor, 95); }
    else for (int i = 0; i < p.stepsPerBar; i += p.grid) setIfEmpty (p, TomFloor, i, 85);
}
void opHalf (Pattern& p)
{
    const int half = p.grid * 2;
    for (int pc : { (int) Snare, (int) SideStick })
    {
        auto& l = p.lines[(size_t) pc];
        if (l.empty()) continue;
        bool had = false;
        for (auto& v : l) if (v >= 90) { v = 0; had = true; }
        if (had) setIfEmpty (p, pc, half, 118);
    }
    if (! p.uses (Snare) && ! p.uses (SideStick)) setIfEmpty (p, Snare, half, 118);
}
void opSnarePush (Pattern& p)
{
    const std::vector<std::pair<int, int>> a = p.grid == 4 ? std::vector<std::pair<int, int>> { { 14, 85 }, { 11, 60 } }
                                                           : std::vector<std::pair<int, int>> { { 11, 85 }, { 8, 60 } };
    for (auto [st, v] : a) if (! stepUsed (p, Kick, st)) setIfEmpty (p, Snare, st, v);
}
void opCross (Pattern& p)
{
    auto& l = p.lines[Snare];
    if (l.empty()) return;
    auto& x = lineOf (p, SideStick);
    for (size_t i = 0; i < l.size() && i < x.size(); ++i)
        if (l[i] >= 55) { x[i] = juce::jmax (x[i], l[i]); l[i] = 0; }
}
void opClap (Pattern& p)
{
    const auto& src = ! p.lines[Snare].empty() ? p.lines[Snare] : p.lines[SideStick];
    if (src.empty()) return;
    bool any = false;
    std::vector<size_t> hits;
    for (size_t i = 0; i < src.size(); ++i) if (src[i] >= 90) hits.push_back (i);
    if (! hits.empty()) { auto& c = lineOf (p, Clap); for (auto i : hits) { c[i] = 100; any = true; } }
    if (! any) for (int st : { p.grid, p.grid * 3 }) setIfEmpty (p, Clap, st, 100);
}
void opBell (Pattern& p) { for (int i = 0; i < p.stepsPerBar; i += p.grid) setIfEmpty (p, RideBell, i, 92); }
void opBar2 (Pattern& p)
{
    if (p.coreBars == 1)
    {
        for (auto& l : p.lines) if (! l.empty()) { auto copy = l; l.insert (l.end(), copy.begin(), copy.end()); }
        p.coreBars = 2;
    }
    const int b = p.stepsPerBar * (p.coreBars - 1), st = p.stepsPerBar - p.grid;
    for (int i = st; i < p.stepsPerBar; ++i)
    {
        for (int h : { (int) HHClosed, (int) HHOpen, (int) Ride })
            if (! p.lines[(size_t) h].empty()) p.lines[(size_t) h][(size_t) (b + i)] = 0;
        lineOf (p, Snare)[(size_t) (b + i)] = (juce::uint8) (70 + juce::roundToInt (35.0f * (float) (i - st) / (float) juce::jmax (1, p.grid - 1)));
    }
}
void opQuarterHat (Pattern& p)
{
    for (int h : { (int) HHClosed, (int) Ride, (int) HHOpen })
    {
        auto& l = p.lines[(size_t) h];
        for (size_t i = 0; i < l.size(); ++i) if ((int) i % p.grid != 0) l[i] = 0;
    }
}
void opHat16 (Pattern& p)
{
    const int h = hpOf (p);
    if (h < 0) return;
    for (int i = 0; i < p.stepsPerBar; ++i) if (! stepUsed (p, HHOpen, i)) setIfEmpty (p, h, i, 55);
}

struct Op { const char* name; void (*fn) (Pattern&); const char* group; };
void opColourFn (Pattern& p) { applyVariantColour (p); }
const std::vector<Op>& ops()
{
    static const std::vector<Op> o {
        { "Colore", opColourFn, "Colore" }, { "Plus", applyVariantPlus, "Plus" }, { "Sync", applyVariantSync, "Sync" },
        { "Light", applyVariantLight, "Light" }, { "Ghost", applyVariantGhost, "Ghost" }, { "Perc", applyVariantPerc, "Percussioni" },
        { "Push", applyVariantPush, "Push" },
        { "Four Floor", opFour, "Sync" }, { "Kick A", opKickA, "Sync" }, { "Kick B", opKickB, "Sync" }, { "Kick C", opKickC, "Sync" },
        { "Upbeat", opUpbeat, "Colore" }, { "Open 2&4", opOpen24, "Colore" }, { "Tom Groove", opTom, "Colore" },
        { "Half-Time", opHalf, "Light" }, { "Snare Push", opSnarePush, "Push" }, { "Cross Stick", opCross, "Percussioni" },
        { "Clap", opClap, "Percussioni" }, { "Bell", opBell, "Colore" }, { "Bar 2 Fill", opBar2, "Push" },
        { "Quarter Hat", opQuarterHat, "Light" }, { "16th Hat", opHat16, "Plus" } };
    return o;
}

bool okPair (const juce::String& a, const juce::String& b)
{
    static const juce::StringArray bad { "Light|Plus", "Light|16th Hat", "Quarter Hat|16th Hat", "Plus|Quarter Hat", "Quarter Hat|Upbeat",
                                         "Tom Groove|Quarter Hat", "Tom Groove|16th Hat", "Tom Groove|Upbeat", "Tom Groove|Open 2&4",
                                         "Kick A|Kick B", "Kick A|Kick C", "Kick B|Kick C", "Cross Stick|Clap", "Colore|Tom Groove" };
    return ! bad.contains (a + "|" + b) && ! bad.contains (b + "|" + a);
}

// Stesso generatore della simulazione (mulberry32): stessi numeri di preset, stessi groove
struct Mulberry
{
    juce::uint32 a;
    explicit Mulberry (juce::uint32 seed) : a (seed ? seed : 1u) {}
    double next()
    {
        a += 0x6D2B79F5u;
        juce::uint32 t = (a ^ (a >> 15)) * (a | 1u);
        t = (t + ((t ^ (t >> 7)) * (t | 61u))) ^ t;
        return (double) (t ^ (t >> 14)) / 4294967296.0;
    }
    int nextInt (int n) { return (int) std::floor (next() * n); }
};

template <typename T> std::vector<T> shuffled (std::vector<T> v, juce::uint32 seed)
{
    Mulberry r (seed);
    for (int i = (int) v.size() - 1; i > 0; --i) std::swap (v[(size_t) i], v[(size_t) r.nextInt (i + 1)]);
    return v;
}

struct Combos { std::vector<std::vector<int>> singles, pairs, triples; };
const Combos& combos()
{
    static const Combos c = []
    {
        Combos x;
        const auto& o = ops();
        const int n = (int) o.size();
        for (int i = 7; i < n; ++i) x.singles.push_back ({ i });
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                if (okPair (o[(size_t) i].name, o[(size_t) j].name)) x.pairs.push_back ({ i, j });
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                for (int k = j + 1; k < n; ++k)
                    if (okPair (o[(size_t) i].name, o[(size_t) j].name) && okPair (o[(size_t) i].name, o[(size_t) k].name)
                        && okPair (o[(size_t) j].name, o[(size_t) k].name))
                        x.triples.push_back ({ i, j, k });
        return x;
    }();
    return c;
}

Pattern basePattern (const Core& c, int s, int sub, int v)
{
    Pattern p;
    p.style = s;
    p.sub = sub;
    p.grid = c.grid;
    p.stepsPerBar = c.grid * 4;
    p.id = v < 10 ? (juce::uint32) (s * 1000 + sub * 100 + v * 7 + 11) : (juce::uint32) (500000 + s * 100000 + sub * 1000 + v);
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
        for (int i = 0; i < steps.length(); ++i) line.push_back ((juce::uint8) velFromChar ((char) steps[i]));
        maxLen = juce::jmax (maxLen, (int) line.size());
    }
    p.coreBars = juce::jmax (1, maxLen / p.stepsPerBar);
    for (auto& l : p.lines) if (! l.empty()) l.resize ((size_t) (p.coreBars * p.stepsPerBar), 0);
    return p;
}

void finishPattern (Pattern& p, int s, const Core& c, int v, const juce::String& label, const juce::String& vgroup)
{
    int hits = 0;
    for (auto& l : p.lines) for (auto x : l) if (x > 0) ++hits;
    const float perBar = (float) hits / (float) p.coreBars;
    p.energy = perBar < 16 ? "Leggero" : (perBar < 28 ? "Medio" : "Intenso");
    const auto& sn = p.uses (Snare) ? p.lines[Snare] : p.lines[SideStick];
    bool anyStrong = false, onlyThree = true;
    for (int i = 0; i < p.stepsPerBar && i < (int) sn.size(); ++i)
        if (sn[(size_t) i] >= 90) { anyStrong = true; if (i != p.stepsPerBar / 2) onlyThree = false; }
    p.feel = p.grid == 3 ? "Shuffle" : ((anyStrong && onlyThree) ? "Half-time" : "Dritto");
    p.vgroup = vgroup;
    p.label = label;
    p.name = juce::String (v + 1).paddedLeft ('0', 2) + "  " + label;
    p.fullName = juce::String (PatternLibrary::styleName (s)) + " / " + c.name + " / " + label;
    p.searchText = (p.fullName + " " + p.energy + " " + p.feel + " " + p.vgroup + (p.coreBars > 1 ? " 2 battute" : "")).toLowerCase();
}
} // namespace

PatternLibrary::PatternLibrary()
{
    auto& all = cores();
    patterns.resize ((size_t) NumStyles);
    subs.resize ((size_t) NumStyles);
    static const char* vgroups[] = { "Base", "Colore", "Plus", "Colore", "Sync", "Colore", "Light", "Ghost", "Percussioni", "Push" };

    for (int s = 0; s < NumStyles; ++s)
    {
        auto& styleCores = all[(size_t) s];
        patterns[(size_t) s].resize ((size_t) subsPerStyle);

        for (int sub = 0; sub < subsPerStyle; ++sub)
        {
            const auto& c = styleCores[(size_t) (sub % (int) styleCores.size())];
            subs[(size_t) s].add (c.name);
            std::set<juce::String> seen;
            auto& list = patterns[(size_t) s][(size_t) sub];
            list.reserve ((size_t) presetsPerSub);

            // Preset 1-10: le varianti classiche
            for (int v = 0; v < 10; ++v)
            {
                Pattern p = basePattern (c, s, sub, v);
                juce::String label;
                switch (v)
                {
                    case 0: label = "Base"; break;
                    case 1: label = applyVariantColour (p); break;
                    case 2: applyVariantPlus (p); label = "Plus"; break;
                    case 3: label = applyVariantColour (p) + " Plus"; applyVariantPlus (p); break;
                    case 4: applyVariantSync (p); label = "Sync"; break;
                    case 5: label = "Sync " + applyVariantColour (p); applyVariantSync (p); break;
                    case 6: applyVariantLight (p); label = "Light"; break;
                    case 7: applyVariantGhost (p); label = "Ghost"; break;
                    case 8: applyVariantPerc (p); label = "Perc"; break;
                    default: applyVariantPush (p); label = "Push"; break;
                }
                std::vector<void (*) (Pattern&)> chain;
                if (v == 4 || v == 5) chain.push_back (altKick);
                if (v == 6) chain.push_back (scaleVel);
                if (v == 7) chain.push_back (forceGhost);
                for (auto f : { applyVariantPerc, forceGhost, altKick, applyVariantPush, scaleVel, accentCym }) chain.push_back (f);
                auto key = linesKey (p);
                size_t ci = 0;
                while (seen.count (key) && ci < chain.size()) { chain[ci++] (p); key = linesKey (p); }
                seen.insert (key);
                finishPattern (p, s, c, v, label, vgroups[v]);
                list.push_back (std::move (p));
            }

            // Preset 11-100: combinazioni degli elementi, tutte diverse tra loro
            const auto& cb = combos();
            const auto seed = (juce::uint32) (s * 7919 + sub * 104729 + 13);
            auto cand = cb.singles;
            for (auto& x : shuffled (cb.pairs, seed)) cand.push_back (x);
            for (auto& x : shuffled (cb.triples, seed + 1)) cand.push_back (x);
            int v = 10;
            for (size_t ci = 0; v < presetsPerSub && ci < cand.size(); ++ci)
            {
                const auto& combo = cand[ci];
                Pattern p = basePattern (c, s, sub, v);
                juce::StringArray names;
                for (int oi : combo)
                {
                    const auto& o = ops()[(size_t) oi];
                    if (oi == 0) names.add (applyVariantColour (p));
                    else { o.fn (p); names.add (o.name); }
                }
                const auto key = linesKey (p);
                if (seen.count (key)) continue;
                seen.insert (key);
                finishPattern (p, s, c, v, names.joinIntoString (" + "), ops()[(size_t) combo[0]].group);
                list.push_back (std::move (p));
                ++v;
            }
            jassert ((int) list.size() == presetsPerSub);
        }
    }
}
} // namespace may
