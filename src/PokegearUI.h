#ifndef GUARD_POKEGEAR_UI_H
#define GUARD_POKEGEAR_UI_H

// =============================================================================
//  POKEGEAR MAIN SCREEN - sprite data
//  Four option icons in the plates at the bottom, a glow that slides to the selected
//  plate (hardware alpha blending), and four little dots that show the chosen design.
//  Icons and dots are affine sprites (they pop in and pulse).
// =============================================================================

#include "graphics.h"

// ---- layout (pixels), matches graphics/PokegearBg.png ---------------------------------
#define PG_PLATE_Y        137       // centre of the four plates along the bottom
#define PG_GLOW_Y         138
#define PG_DOT_X0         16        // design dots (top-left): x of the first one
#define PG_DOT_DX         12
#define PG_DOT_Y          14

static const u8 sPgPlateX[POKEGEAR_NUM_OPTIONS] = { 35, 92, 149, 206 };

// ---- tags ---------------------------------------------------------------------------
#define GFXTAG_PG_ICON_PAL     0x7F20
#define GFXTAG_PG_ICON0        0x7F21   // 0x7F21..0x7F24, one per option
#define GFXTAG_PG_ICON_LOCKED  0x7F25   // grey copy of the icon palette (locked options)
#define GFXTAG_PG_GLOW         0x7F26
#define GFXTAG_PG_DOT          0x7F27
#define GFXTAG_PG_DOT_PAL0     0x7F28   // 0x7F28..0x7F2D, one palette per design dot

// callbacks live in Pokegear.c
void PokegearIconCallback(struct Sprite *sprite);
void PokegearGlowCallback(struct Sprite *sprite);
void PokegearDotCallback(struct Sprite *sprite);

// ---- shared animations ----------------------------------------------------------------
static const union AnimCmd sPgAnim_Frame0[] = { ANIMCMD_FRAME(0, 0), ANIMCMD_END };
static const union AnimCmd sPgAnim_Frame1[] = { ANIMCMD_FRAME(16, 0), ANIMCMD_END };   // 32x32 = 16 tiles per frame
static const union AnimCmd *const sPgIconAnims[] = { sPgAnim_Frame0, sPgAnim_Frame1 };

static const union AnimCmd sPgAnim_Still[] = { ANIMCMD_FRAME(0, 0), ANIMCMD_END };
static const union AnimCmd *const sPgStillAnims[] = { sPgAnim_Still };

// Affine sprites: the icons and the design dots are scaled by writing their matrix directly
// (see PgSetSpriteSize in Pokegear.c), so no affine animation tables are needed here.

// ---- OAM -------------------------------------------------------------------------------------
static const struct OamData sPgIconOam =
{
    .affineMode = ST_OAM_AFFINE_DOUBLE,   // double size so the icon can grow without being clipped
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1,
};

static const struct OamData sPgGlowOam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_BLEND,          // semi-transparent: mixed with the background by BLDALPHA
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
    .priority = 2,                        // behind the icons
};

static const struct OamData sPgDotOam =
{
    .affineMode = ST_OAM_AFFINE_DOUBLE,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 1,
};

// ---- sheets + templates -----------------------------------------------------------------------
static const struct SpriteSheet sPgIconSheets[POKEGEAR_NUM_OPTIONS] =
{
    { pg_mapTiles,   32 * 64 / 2, GFXTAG_PG_ICON0 + 0 },
    { pg_radioTiles, 32 * 64 / 2, GFXTAG_PG_ICON0 + 1 },
    { pg_phoneTiles, 32 * 64 / 2, GFXTAG_PG_ICON0 + 2 },
    { pg_exitTiles,  32 * 64 / 2, GFXTAG_PG_ICON0 + 3 },
};

#define PG_ICON_TEMPLATE(n) \
    { .tileTag = GFXTAG_PG_ICON0 + (n), .paletteTag = GFXTAG_PG_ICON_PAL, .oam = &sPgIconOam, \
      .anims = sPgIconAnims, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = PokegearIconCallback }

static const struct SpriteTemplate sPgIconTemplates[POKEGEAR_NUM_OPTIONS] =
{
    PG_ICON_TEMPLATE(0), PG_ICON_TEMPLATE(1), PG_ICON_TEMPLATE(2), PG_ICON_TEMPLATE(3),
};

static const struct SpriteSheet sPgGlowSheet = { pg_glowTiles, 64 * 32 / 2, GFXTAG_PG_GLOW };
static const struct SpritePalette sPgGlowPalette = { pg_glowPal, GFXTAG_PG_GLOW };

static const struct SpriteTemplate sPgGlowTemplate =
{
    .tileTag = GFXTAG_PG_GLOW,
    .paletteTag = GFXTAG_PG_GLOW,
    .oam = &sPgGlowOam,
    .anims = sPgStillAnims,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = PokegearGlowCallback,
};

static const struct SpriteSheet sPgDotSheet = { pg_pipTiles, 8 * 8 / 2, GFXTAG_PG_DOT };

#define PG_DOT_TEMPLATE(n) \
    { .tileTag = GFXTAG_PG_DOT, .paletteTag = GFXTAG_PG_DOT_PAL0 + (n), .oam = &sPgDotOam, \
      .anims = sPgStillAnims, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = PokegearDotCallback }

static const struct SpriteTemplate sPgDotTemplates[POKEGEAR_NUM_DESIGNS] =
{
    PG_DOT_TEMPLATE(0), PG_DOT_TEMPLATE(1), PG_DOT_TEMPLATE(2), PG_DOT_TEMPLATE(3),
    PG_DOT_TEMPLATE(4), PG_DOT_TEMPLATE(5),
};

#endif // GUARD_POKEGEAR_UI_H
