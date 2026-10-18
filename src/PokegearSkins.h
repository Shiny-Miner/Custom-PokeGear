#ifndef GUARD_POKEGEAR_SKINS_H
#define GUARD_POKEGEAR_SKINS_H

// =============================================================================
//  POKEGEAR DESIGNS (colour themes)
//  The main screen, the phone card and the radio card all use the design that was
//  chosen on the main screen (L / R). A design is just a set of 16 colours for each
//  of the three backgrounds, in the colour order of its PNG
//  (graphics/PokegearBg.png, PhoneCardBg.png, RadioCardBg.png).
//  The gold trim, white, black and greys are the same in every design.
//  To change a design, edit its colours: PG_RGB(r, g, b) with r, g, b in 0..31.
// =============================================================================

#define POKEGEAR_NUM_DESIGNS 6

#define PG_RGB(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

// ---- main screen (graphics/PokegearBg.png) ----
static const u16 sPokegearSkinPal[POKEGEAR_NUM_DESIGNS][16] =
{
    { // Red
        PG_RGB(18,  0,  2),  // 0: #900010
        PG_RGB(10,  4,  5),  // 1: #522129
        PG_RGB(31, 31, 31),  // 2: #F8F8F8
        PG_RGB(30, 22,  6),  // 3: #F7B531
        PG_RGB(30, 22,  6),  // 4: #F0B030
        PG_RGB(23,  8, 10),  // 5: #B84050
        PG_RGB(15,  1,  3),  // 6: #7B0818
        PG_RGB(13,  2,  4),  // 7: #6B1021
        PG_RGB(14,  2,  4),  // 8: #701020
        PG_RGB(26, 15,  4),  // 9: #D07820
        PG_RGB(24, 11,  4),  // 10: #C05820
        PG_RGB(28, 17,  4),  // 11: #E08820
        PG_RGB(29, 20,  5),  // 12: #E8A028
        PG_RGB(26, 15,  4),  // 13: #D67B21
        PG_RGB(22, 19, 19),  // 14: #B09898
        PG_RGB( 0,  0,  0),  // 15: #000000
    },
    { // Blue
        PG_RGB( 0,  8, 20),  // 0: #0041A6
        PG_RGB( 4,  7, 11),  // 1: #263F5E
        PG_RGB(31, 31, 31),  // 2: #F8F8F8
        PG_RGB(30, 22,  6),  // 3: #F7B531
        PG_RGB(30, 22,  6),  // 4: #F0B030
        PG_RGB( 9, 16, 26),  // 5: #4A83D4
        PG_RGB( 1,  8, 17),  // 6: #09418D
        PG_RGB( 2,  8, 15),  // 7: #12437B
        PG_RGB( 2,  8, 16),  // 8: #124481
        PG_RGB(26, 15,  4),  // 9: #D07820
        PG_RGB(24, 11,  4),  // 10: #C05820
        PG_RGB(28, 17,  4),  // 11: #E08820
        PG_RGB(29, 20,  5),  // 12: #E8A028
        PG_RGB(26, 15,  4),  // 13: #D67B21
        PG_RGB(21, 22, 25),  // 14: #AFB7CA
        PG_RGB( 0,  0,  0),  // 15: #000000
    },
    { // Green
        PG_RGB( 3, 17,  0),  // 0: #188907
        PG_RGB( 5,  9,  4),  // 1: #2A4E22
        PG_RGB(31, 31, 31),  // 2: #F8F8F8
        PG_RGB(30, 22,  6),  // 3: #F7B531
        PG_RGB(30, 22,  6),  // 4: #F0B030
        PG_RGB(10, 21,  8),  // 5: #53AF42
        PG_RGB( 3, 14,  1),  // 6: #1E750D
        PG_RGB( 4, 12,  2),  // 7: #256614
        PG_RGB( 4, 13,  2),  // 8: #246A14
        PG_RGB(26, 15,  4),  // 9: #D07820
        PG_RGB(24, 11,  4),  // 10: #C05820
        PG_RGB(28, 17,  4),  // 11: #E08820
        PG_RGB(29, 20,  5),  // 12: #E8A028
        PG_RGB(26, 15,  4),  // 13: #D67B21
        PG_RGB(18, 20, 18),  // 14: #92A792
        PG_RGB( 0,  0,  0),  // 15: #000000
    },
    { // Yellow
        PG_RGB(18, 16,  0),  // 0: #978000
        PG_RGB(10,  9,  4),  // 1: #564C23
        PG_RGB(31, 31, 31),  // 2: #F8F8F8
        PG_RGB(30, 22,  6),  // 3: #F7B531
        PG_RGB(30, 22,  6),  // 4: #F0B030
        PG_RGB(24, 21,  8),  // 5: #C1AB43
        PG_RGB(16, 13,  1),  // 6: #816C08
        PG_RGB(14, 11,  2),  // 7: #705B11
        PG_RGB(14, 12,  2),  // 8: #766111
        PG_RGB(26, 15,  4),  // 9: #D07820
        PG_RGB(24, 11,  4),  // 10: #C05820
        PG_RGB(28, 17,  4),  // 11: #E08820
        PG_RGB(29, 20,  5),  // 12: #E8A028
        PG_RGB(26, 15,  4),  // 13: #D67B21
        PG_RGB(23, 23, 20),  // 14: #B9B8A0
        PG_RGB( 0,  0,  0),  // 15: #000000
    },
    { // Purple
        PG_RGB(11,  1, 18),  // 0: #5D0F97
        PG_RGB( 8,  5, 10),  // 1: #402856
        PG_RGB(31, 31, 31),  // 2: #F8F8F8
        PG_RGB(30, 22,  6),  // 3: #F7B531
        PG_RGB(30, 22,  6),  // 4: #F0B030
        PG_RGB(17, 10, 24),  // 5: #8E50C1
        PG_RGB( 9,  2, 16),  // 6: #4F1481
        PG_RGB( 8,  3, 14),  // 7: #451A70
        PG_RGB( 9,  3, 14),  // 8: #491B76
        PG_RGB(26, 15,  4),  // 9: #D07820
        PG_RGB(24, 11,  4),  // 10: #C05820
        PG_RGB(28, 17,  4),  // 11: #E08820
        PG_RGB(29, 20,  5),  // 12: #E8A028
        PG_RGB(26, 15,  4),  // 13: #D67B21
        PG_RGB(22, 20, 23),  // 14: #B2A2B9
        PG_RGB( 0,  0,  0),  // 15: #000000
    },
    { // Pink
        PG_RGB(20,  3, 12),  // 0: #A61964
        PG_RGB(11,  5,  9),  // 1: #5E2E4A
        PG_RGB(31, 31, 31),  // 2: #F8F8F8
        PG_RGB(30, 22,  6),  // 3: #F7B531
        PG_RGB(30, 22,  6),  // 4: #F0B030
        PG_RGB(26, 11, 19),  // 5: #D45E9F
        PG_RGB(17,  3, 11),  // 6: #8D1D5C
        PG_RGB(15,  4, 11),  // 7: #7B2258
        PG_RGB(16,  4, 11),  // 8: #81235A
        PG_RGB(26, 15,  4),  // 9: #D07820
        PG_RGB(24, 11,  4),  // 10: #C05820
        PG_RGB(28, 17,  4),  // 11: #E08820
        PG_RGB(29, 20,  5),  // 12: #E8A028
        PG_RGB(26, 15,  4),  // 13: #D67B21
        PG_RGB(25, 22, 23),  // 14: #CAB3BD
        PG_RGB( 0,  0,  0),  // 15: #000000
    },
};

// ---- phone card (graphics/PhoneCardBg.png) ----
static const u16 sPhoneCardSkinPal[POKEGEAR_NUM_DESIGNS][16] =
{
    { // Red
        PG_RGB(20,  1,  4),  // 0: #A00F20
        PG_RGB(17,  1,  3),  // 1: #8C0A1A
        PG_RGB(29, 22,  4),  // 2: #E8B020
        PG_RGB(21, 14,  1),  // 3: #A8760C
        PG_RGB( 2,  2,  2),  // 4: #101010
        PG_RGB(31, 31, 31),  // 5: #FFFFFF
        PG_RGB(13,  0,  2),  // 6: #6E0014
        PG_RGB( 7,  6,  6),  // 7: #3C3030
        PG_RGB( 9,  7,  7),  // 8: #4A3C3C
        PG_RGB(11, 11, 13),  // 9: #585A6C
        PG_RGB(12, 13, 15),  // 10: #66687A
        PG_RGB( 4,  3,  3),  // 11: #241C1C
        PG_RGB(26, 26, 27),  // 12: #D4D4DC
        PG_RGB(17, 17, 18),  // 13: #8A8A96
        PG_RGB(10,  0,  1),  // 14: #54000E
        PG_RGB(22, 22, 23),  // 15: #B0B0BC
    },
    { // Blue
        PG_RGB( 2, 10, 23),  // 0: #1153B8
        PG_RGB( 1,  9, 20),  // 1: #0B48A1
        PG_RGB(29, 22,  4),  // 2: #E8B020
        PG_RGB(21, 14,  1),  // 3: #A8760C
        PG_RGB( 2,  2,  2),  // 4: #101010
        PG_RGB(31, 31, 31),  // 5: #FFFFFF
        PG_RGB( 0,  7, 15),  // 6: #003A7E
        PG_RGB( 6,  7,  8),  // 7: #373B45
        PG_RGB( 8,  9, 10),  // 8: #454A55
        PG_RGB(11, 11, 13),  // 9: #585A6C
        PG_RGB(12, 13, 15),  // 10: #66687A
        PG_RGB( 4,  4,  5),  // 11: #202329
        PG_RGB(26, 26, 27),  // 12: #D4D4DC
        PG_RGB(17, 17, 18),  // 13: #8A8A96
        PG_RGB( 0,  5, 12),  // 14: #002B61
        PG_RGB(22, 22, 23),  // 15: #B0B0BC
    },
    { // Green
        PG_RGB( 4, 19,  2),  // 0: #279815
        PG_RGB( 4, 16,  2),  // 1: #208510
        PG_RGB(29, 22,  4),  // 2: #E8B020
        PG_RGB(21, 14,  1),  // 3: #A8760C
        PG_RGB( 2,  2,  2),  // 4: #101010
        PG_RGB(31, 31, 31),  // 5: #FFFFFF
        PG_RGB( 3, 13,  0),  // 6: #196805
        PG_RGB( 5,  7,  5),  // 7: #2E392E
        PG_RGB( 7,  8,  7),  // 8: #3A463A
        PG_RGB(11, 11, 13),  // 9: #585A6C
        PG_RGB(12, 13, 15),  // 10: #66687A
        PG_RGB( 3,  4,  3),  // 11: #1B221B
        PG_RGB(26, 26, 27),  // 12: #D4D4DC
        PG_RGB(17, 17, 18),  // 13: #8A8A96
        PG_RGB( 2, 10,  0),  // 14: #125004
        PG_RGB(22, 22, 23),  // 15: #B0B0BC
    },
    { // Yellow
        PG_RGB(21, 18,  2),  // 0: #A89010
        PG_RGB(18, 15,  1),  // 1: #937D0A
        PG_RGB(29, 22,  4),  // 2: #E8B020
        PG_RGB(21, 14,  1),  // 3: #A8760C
        PG_RGB( 2,  2,  2),  // 4: #101010
        PG_RGB(31, 31, 31),  // 5: #FFFFFF
        PG_RGB(14, 11,  0),  // 6: #745A00
        PG_RGB( 7,  7,  6),  // 7: #3F3E32
        PG_RGB( 9,  9,  7),  // 8: #4E4D3F
        PG_RGB(11, 11, 13),  // 9: #585A6C
        PG_RGB(12, 13, 15),  // 10: #66687A
        PG_RGB( 4,  4,  3),  // 11: #26251D
        PG_RGB(26, 26, 27),  // 12: #D4D4DC
        PG_RGB(17, 17, 18),  // 13: #8A8A96
        PG_RGB(11,  8,  0),  // 14: #584600
        PG_RGB(22, 22, 23),  // 15: #B0B0BC
    },
    { // Purple
        PG_RGB(13,  3, 21),  // 0: #6C1FA8
        PG_RGB(11,  3, 18),  // 1: #5D1893
        PG_RGB(29, 22,  4),  // 2: #E8B020
        PG_RGB(21, 14,  1),  // 3: #A8760C
        PG_RGB( 2,  2,  2),  // 4: #101010
        PG_RGB(31, 31, 31),  // 5: #FFFFFF
        PG_RGB( 7,  1, 14),  // 6: #3F0C74
        PG_RGB( 7,  6,  7),  // 7: #3B343F
        PG_RGB( 9,  8,  9),  // 8: #49404E
        PG_RGB(11, 11, 13),  // 9: #585A6C
        PG_RGB(12, 13, 15),  // 10: #66687A
        PG_RGB( 4,  3,  4),  // 11: #231E26
        PG_RGB(26, 26, 27),  // 12: #D4D4DC
        PG_RGB(17, 17, 18),  // 13: #8A8A96
        PG_RGB( 6,  1, 11),  // 14: #320958
        PG_RGB(22, 22, 23),  // 15: #B0B0BC
    },
    { // Pink
        PG_RGB(23,  5, 14),  // 0: #B82A76
        PG_RGB(20,  4, 12),  // 1: #A12267
        PG_RGB(29, 22,  4),  // 2: #E8B020
        PG_RGB(21, 14,  1),  // 3: #A8760C
        PG_RGB( 2,  2,  2),  // 4: #101010
        PG_RGB(31, 31, 31),  // 5: #FFFFFF
        PG_RGB(15,  2, 10),  // 6: #7E1354
        PG_RGB( 8,  7,  7),  // 7: #45393E
        PG_RGB(10,  8,  9),  // 8: #55474D
        PG_RGB(11, 11, 13),  // 9: #585A6C
        PG_RGB(12, 13, 15),  // 10: #66687A
        PG_RGB( 5,  4,  4),  // 11: #292225
        PG_RGB(26, 26, 27),  // 12: #D4D4DC
        PG_RGB(17, 17, 18),  // 13: #8A8A96
        PG_RGB(12,  1,  7),  // 14: #610E3F
        PG_RGB(22, 22, 23),  // 15: #B0B0BC
    },
};

// ---- radio card (graphics/RadioCardBg.png) ----
// (the radio background uses the same colour order as the phone card, so it shares the table)
#define sRadioCardSkinPal sPhoneCardSkinPal

// colour of the little design dots on the main screen (one per design)
static const u16 sPokegearSkinDot[POKEGEAR_NUM_DESIGNS] =
{
    PG_RGB(22, 11, 12), // Red
    PG_RGB(11, 16, 24), // Blue
    PG_RGB(13, 22, 11), // Green
    PG_RGB(23, 21, 11), // Yellow
    PG_RGB(18, 12, 23), // Purple
    PG_RGB(24, 13, 19), // Pink
};

// =============================================================================
//  Settings
// =============================================================================

// ---- which cards can be opened ------------------------------------------------
// A plate is usable once its flag is set (0 = always usable). A locked plate is shown
// greyed out and the cursor skips it. Example: a card that opens with the 2nd badge:
//     #define POKEGEAR_FLAG_PHONE  FLAG_BADGE02_GET
#define POKEGEAR_FLAG_MAP     0
#define POKEGEAR_FLAG_RADIO   0
#define POKEGEAR_FLAG_PHONE   0

// ---- design memory ------------------------------------------------------------
// Optional: a VAR that remembers the chosen design between visits (and after saving).
// 0 = don't remember (the Pokegear opens in design 0; the cards always follow the
// design that was chosen when they were opened).
#define POKEGEAR_DESIGN_VAR   0

// Frames the colours need to fade from one design to the next (60 = 1 second).
#define POKEGEAR_DESIGN_FADE_FRAMES  24

#endif // GUARD_POKEGEAR_SKINS_H
