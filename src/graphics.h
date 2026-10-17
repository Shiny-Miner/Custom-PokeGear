#include "../include/global.h" 
#include "../include/sprite.h" 
#include "../include/bg.h" 

enum GfxTags
{
  GFXTAG_PANEL,
  GFXTAG_MAPCARD,
  GFXTAG_PHONECARD,
  GFXTAG_SCROLLBAR,
  GFXTAG_ARROW_UP,
  GFXTAG_ARROW_DOWN,
  GFXTAG_RADIOCARD,
};



//Gfx Data
extern const u8 panelTiles[]; 
extern const u16 panelPal[];  

// BG Gfx data
extern const u8 PokegearBgTiles[]; 
extern const u8 PokegearBgMap[];
extern const u16 PokegearBgPal[];

// Options Icon Gfx data
extern const u8 TownMapTiles[];
extern const u16 TownMapPal[];

// Scrollbar Gfx data
extern const u8 scrollbarTiles[];
extern const u16 scrollbarPal[];

// Phone card
extern const u8 phonecardTiles[];
extern const u16 phonecardPal[];

// Phone card bg
extern const u8 PhoneCardBgTiles[]; 
extern const u16 PhoneCardBgPal[]; 
extern const u8 PhoneCardBgMap[];
// arrow_up
extern const u8 arrow_upTiles[];
extern const u16 arrow_upPal[];
#define GFXTAG_ARROW_UP 0x7F10

// arrow_down
extern const u8 arrow_downTiles[];
extern const u16 arrow_downPal[];
#define GFXTAG_ARROW_DOWN 0x7F11

// arrow (phone card cursor)
extern const u8 arrowTiles[];
extern const u16 arrowPal[];
#define GFXTAG_PHONE_CURSOR   0x7F12

// Radio card
extern const u8 radiocardTiles[];
extern const u16 radiocardPal[];
extern const u8 RadioCardBgTiles[];
extern const u16 RadioCardBgPal[];
extern const u8 RadioCardBgMap[];
extern const u8 radio_needleTiles[];
extern const u16 radio_needlePal[];
#define GFXTAG_RADIO_NEEDLE   0x7F14
extern const u8 radio_markerTiles[];
extern const u16 radio_markerPal[];
#define GFXTAG_RADIO_MARKER   0x7F15

// phone card portrait (tiles come from the overworld sprite table)
#define GFXTAG_PHONE_PORTRAIT 0x7F13



//================================================================================================
void PanelCallBack(struct Sprite *sprite);
void PokegearIconCallback(struct Sprite *sprite);
void ScrollBarCallback(struct Sprite *sprite);

//=================================================================================================
static const struct SpriteSheet PanelSpriteSheet = {panelTiles, (64*64*4)/2, GFXTAG_PANEL };
static const struct SpritePalette PanelSpritePalette = {panelPal, GFXTAG_PANEL};

static const struct OamData sPanelOam =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_BLEND,
	.shape = SPRITE_SHAPE(64x64),
	.size = SPRITE_SIZE(64x64),
	.priority = 1, //Above other sprites
};

static const union AnimCmd sAnimCmdPanelUnselected1[] =
{
	ANIMCMD_FRAME(0, 0),
	ANIMCMD_END
}; 

static const union AnimCmd sAnimCmdPanelUnselected2[] =
{
	ANIMCMD_FRAME(64, 0),
	ANIMCMD_END
};  
static const union AnimCmd sAnimCmdPanelSelected1[] =
{
	ANIMCMD_FRAME(128, 0),
	ANIMCMD_END
}; 

static const union AnimCmd sAnimCmdPanelSelected2[] =
{
	ANIMCMD_FRAME(192, 0),       
	ANIMCMD_JUMP(0) 
};  


static const union AnimCmd *const sAnimCmdTable_Panel1[] =
{
	sAnimCmdPanelUnselected1,
	sAnimCmdPanelSelected1,
	
}; 

static const union AnimCmd *const sAnimCmdTable_Panel2[] =
{
	sAnimCmdPanelUnselected2,
	sAnimCmdPanelSelected2,
	
};  

static const struct SpriteTemplate sPanel1SpriteTemplate =
{
	.tileTag = GFXTAG_PANEL,
	.paletteTag = GFXTAG_PANEL,
	.oam = &sPanelOam,
	.anims = sAnimCmdTable_Panel1,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = PanelCallBack,
}; 
static const struct SpriteTemplate sPanel2SpriteTemplate =
{
	.tileTag = GFXTAG_PANEL,
	.paletteTag = GFXTAG_PANEL,
	.oam = &sPanelOam,
	.anims = sAnimCmdTable_Panel2,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = PanelCallBack,
}; 

// Pokedex Sprite

static const struct OamData sIconOamData =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_NORMAL,
	.shape = SPRITE_SHAPE(32x32),
	.size = SPRITE_SIZE(32x32),
	.priority = 1, //Above other sprites
};
static const struct OamData ScrollBarOamData =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_NORMAL,
	.shape = SPRITE_SHAPE(32x32),
	.size = SPRITE_SIZE(32x32),
	.priority = 1, //Above other sprites
};

static const union AnimCmd sAnimCmdIcon[] =
{
	ANIMCMD_FRAME(0, 0),
	ANIMCMD_END
};
static const union AnimCmd sAnimCmdIconSelected[] =
{
	ANIMCMD_FRAME(16, 0),
	ANIMCMD_END
}; 
static const union AnimCmd *const sAnimCmdTable_Icon[] =
{
	sAnimCmdIcon,
	sAnimCmdIconSelected
};   

static const struct SpriteSheet ScrollBarSpriteSheet = {scrollbarTiles, 32*16, GFXTAG_SCROLLBAR};
static const struct SpritePalette ScrollBarSpritePalette = {scrollbarPal, GFXTAG_SCROLLBAR};

static const union AnimCmd AnimCmdScrollbar[] = 
{
  ANIMCMD_FRAME(0, 0),
  ANIMCMD_END,
}; 
static const union AnimCmd *const sAnimCmdTable_Scrollbar[] =
{
	AnimCmdScrollbar
};   


static const struct SpriteTemplate ScrollBarSpriteTemplate =
{
  .tileTag = GFXTAG_SCROLLBAR,
	.paletteTag = GFXTAG_SCROLLBAR,
	.oam = &ScrollBarOamData,
	.anims = sAnimCmdTable_Scrollbar,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = ScrollBarCallback,
};
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

