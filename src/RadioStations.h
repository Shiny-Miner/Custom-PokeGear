#ifndef GUARD_RADIO_STATIONS_H
#define GUARD_RADIO_STATIONS_H

// =============================================================================
//  RADIO CARD STATION TABLE  -  the only file you need to edit to add or change
//  radio stations.
//
//  FREQUENCY   in tenths of MHz: 893 = 89.3 MHz. The dial covers
//              RADIO_FREQ_MIN .. RADIO_FREQ_MAX. Keep stations at least ~1 MHz apart
//              (a station is received within +-RADIO_LOCK_RANGE of its frequency).
//  UNLOCK FLAG the station only exists on the dial once this flag is set
//              (PHONE_ALWAYS = from the start). Set it from a script with setflag.
//  SONG        MUS_* constant played while tuned in (0 = no music).
//  PORTRAIT    graphicsId = any index of your overworld sprite table (the DJ).
//  PROGRAM     list of lines; every line whose flag is set is part of the rotation
//              (PHONE_ALWAYS = always). Lines play in order, then loop, so you can
//              make the program change with the game state just by adding flagged
//              lines. A line may use \n for a second row (2 rows of ~24 characters).
//
//  TEXT        All text lives in strings/PokeGear.string (#org @label). Declare each
//              label in the "text labels" list below.
// =============================================================================

#include "../include/constants/songs.h"

// ---- dial / behaviour settings -------------------------------------------------
#define RADIO_FREQ_MIN      880     // 88.0 MHz
#define RADIO_FREQ_MAX      1080    // 108.0 MHz
#define RADIO_DIAL_X0       20      // x of RADIO_FREQ_MIN on the dial (matches RadioCardBg.png)
#define RADIO_DIAL_W        200     // dial width in pixels (RADIO_FREQ_MAX - RADIO_FREQ_MIN steps of 0.1 -> 1 px each)
#define RADIO_LOCK_RANGE    3       // station is received within +-0.3 MHz
#define RADIO_LINE_FRAMES   150     // frames a finished line stays on screen (60 = 1 second)

// Optional: a VAR that remembers the last tuned frequency between visits.
// 0 = don't remember (the radio opens on the first station).
#define RADIO_FREQ_VAR      0

// ---- station data types ---------------------------------------------------------
struct RadioLine
{
    u16 flag;               // line is played when this flag is set (or PHONE_ALWAYS)
    const u8* text;
};

struct RadioStation
{
    const u8* name;
    u16 freq;               // tenths of MHz
    u16 unlockFlag;         // station exists once this flag is set (or PHONE_ALWAYS)
    u16 song;               // MUS_* played while tuned in (0 = none)
    u16 graphicsId;         // DJ portrait (overworld sprite table index)
    const struct RadioLine* lines;
    u8 lineCount;
};

// ---- text labels (defined in strings/PokeGear.string) ------------------------------
extern const u8 gText_RadioName1[];
extern const u8 gText_RadioName2[];
extern const u8 gText_RadioName3[];
extern const u8 gText_RadioName4[];
extern const u8 gText_RadioTalk1[];
extern const u8 gText_RadioTalk2[];
extern const u8 gText_RadioTalk3[];
extern const u8 gText_RadioTalk4[];
extern const u8 gText_RadioMusic1[];
extern const u8 gText_RadioMusic2[];
extern const u8 gText_RadioMusic3[];
extern const u8 gText_RadioFlute1[];
extern const u8 gText_RadioFlute2[];
extern const u8 gText_RadioLucky1[];
extern const u8 gText_RadioLucky2[];

// ---- programs ----------------------------------------------------------------------
static const struct RadioLine sRadioLines_Talk[] =
{
    { PHONE_ALWAYS,     gText_RadioTalk1 },
    { PHONE_ALWAYS,     gText_RadioTalk2 },
    { PHONE_ALWAYS,     gText_RadioTalk3 },
    { FLAG_BADGE01_GET, gText_RadioTalk4 },     // example: only after the first badge
};

static const struct RadioLine sRadioLines_Music[] =
{
    { PHONE_ALWAYS,        gText_RadioMusic1 },
    { PHONE_ALWAYS,        gText_RadioMusic2 },
    { FLAG_SYS_GAME_CLEAR, gText_RadioMusic3 }, // example: only after becoming Champion
};

static const struct RadioLine sRadioLines_Flute[] =
{
    { PHONE_ALWAYS,     gText_RadioFlute1 },
    { PHONE_ALWAYS,     gText_RadioFlute2 },
};

static const struct RadioLine sRadioLines_Lucky[] =
{
    { PHONE_ALWAYS,     gText_RadioLucky1 },
    { PHONE_ALWAYS,     gText_RadioLucky2 },
};

#define RADIO_LINES(table) table, ARRAY_COUNT(table)

// ---- the station table (order does not matter: the dial sorts by frequency) --------------
//  name,             freq, unlock flag,           song,            DJ portrait,            program
static const struct RadioStation sRadioStations[] =
{
    { gText_RadioName1,  893, PHONE_ALWAYS,        MUS_PALLET,      OBJ_EVENT_GFX_WOMAN_1,  RADIO_LINES(sRadioLines_Talk)  },
    { gText_RadioName2,  925, FLAG_SYS_POKEDEX_GET, MUS_ROUTE1,     OBJ_EVENT_GFX_BEAUTY,   RADIO_LINES(sRadioLines_Music) },
    { gText_RadioName3,  968, FLAG_GOT_POKE_FLUTE, MUS_CYCLING,     OBJ_EVENT_GFX_HIKER,    RADIO_LINES(sRadioLines_Flute) },
    { gText_RadioName4, 1013, FLAG_SYS_GAME_CLEAR, MUS_GAME_CORNER, OBJ_EVENT_GFX_GENTLEMAN, RADIO_LINES(sRadioLines_Lucky) },
};

#endif // GUARD_RADIO_STATIONS_H
