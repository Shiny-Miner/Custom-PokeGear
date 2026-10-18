#ifndef GUARD_POKEGEAR_GRAPHICS_H
#define GUARD_POKEGEAR_GRAPHICS_H

#include "../include/global.h" 
#include "../include/sprite.h" 
#include "../include/bg.h" 

// ---------------------------------------------------------------------------
// BG graphics
// ---------------------------------------------------------------------------
// Pokegear main screen
extern const u8 PokegearBgTiles[]; 
extern const u8 PokegearBgMap[];
extern const u16 PokegearBgPal[];   // = design 0; the other designs live in PokegearSkins.h

// Phone card
extern const u8 PhoneCardBgTiles[]; 
extern const u16 PhoneCardBgPal[]; 
extern const u8 PhoneCardBgMap[];

// Radio card
extern const u8 RadioCardBgTiles[];
extern const u16 RadioCardBgPal[];
extern const u8 RadioCardBgMap[];

// ---------------------------------------------------------------------------
// Sprite graphics
// ---------------------------------------------------------------------------
// Main screen: option icons (32x64, 2 frames), selection glow, design dots
extern const u8 pg_mapTiles[];
extern const u16 pg_mapPal[];
extern const u8 pg_radioTiles[];
extern const u16 pg_radioPal[];
extern const u8 pg_phoneTiles[];
extern const u16 pg_phonePal[];
extern const u8 pg_exitTiles[];
extern const u16 pg_exitPal[];
extern const u8 pg_glowTiles[];
extern const u16 pg_glowPal[];
extern const u8 pg_pipTiles[];
extern const u16 pg_pipPal[];

// Phone card: scroll arrows
extern const u8 arrow_upTiles[];
extern const u16 arrow_upPal[];
#define GFXTAG_ARROW_UP 0x7F10

extern const u8 arrow_downTiles[];
extern const u16 arrow_downPal[];
#define GFXTAG_ARROW_DOWN 0x7F11

// Phone card portrait (tiles come from the overworld sprite table)
#define GFXTAG_PHONE_PORTRAIT 0x7F13

// Radio card: tuner needle + station markers
extern const u8 radio_needleTiles[];
extern const u16 radio_needlePal[];
#define GFXTAG_RADIO_NEEDLE   0x7F14
extern const u8 radio_markerTiles[];
extern const u16 radio_markerPal[];
#define GFXTAG_RADIO_MARKER   0x7F15

static const struct OamData sArrowOam =
{
    .shape = SPRITE_SHAPE(16x16),   // arrow_up.png / arrow_down.png are 16x16
    .size = SPRITE_SIZE(16x16),
    .priority = 0,
};

static const union AnimCmd sAnim_ArrowStill[] = {
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sAnimTable_Arrow[] = {
    sAnim_ArrowStill,
};
static const struct SpriteTemplate sSpriteTemplate_ArrowUp =
{
    .tileTag = GFXTAG_ARROW_UP,
    .paletteTag = GFXTAG_ARROW_UP,
    .oam = &sArrowOam,
    .anims = sAnimTable_Arrow,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_ArrowDown =
{
    .tileTag = GFXTAG_ARROW_DOWN,
    .paletteTag = GFXTAG_ARROW_DOWN,
    .oam = &sArrowOam,
    .anims = sAnimTable_Arrow,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

#endif // GUARD_POKEGEAR_GRAPHICS_H
