#ifndef GUARD_Pokegear_H
#define GUARD_Pokegear_H

#include "../include/global.h" 
#include "../include/sprite.h" 
#include "../include/bg.h" 
#include "../include/window.h" 
#include "../include/text.h"
#include "graphics.h"

// Overworld sprite table access (symbols come from BPRE.ld)
extern const struct EventObjectGraphicsInfo* GetEventObjectGraphicsInfoOriginal(u16 graphicsId);
extern void MakeObjectTemplateFromEventObjectGraphicsInfo(u16 graphicsId, void (*callback)(struct Sprite *), struct SpriteTemplate *spriteTemplate, const struct SubspriteTable **subspriteTables);
extern void PatchObjectPalette(u16 paletteTag, u8 paletteSlot);
extern u8* GetMapName(u8* dest, u16 regionMapId, u16 padLength);
extern u8  GetCurrentRegionMapSectionId(void); 
void CB2_ReturnToField(void);
void FieldCB_WarpExitFadeFromBlack(void);


enum BGs
{
	BG_TEXT,
	BG_1,
	BG_2,
	BG_BACKGROUND,
}; 

enum PokegearOptions
{
  Pokegear_MAPCARD = 0,
  Pokegear_RADIOCARD,
  Pokegear_PHONECARD,
  Pokegear_EXIT,
  POKEGEAR_NUM_OPTIONS
};

struct PokegearOption 
{
  u8 id;
  u8 * text;
  u16 flag; 
  u8 * script;
  void (*func);
};

// Text
extern const u8 gText_Pokegear_MapCard[];
extern const u8 gText_Pokegear_PhoneCard[];
extern const u8 gText_Pokegear_RadioCard[];
extern const u8 gText_Pokegear_Exit[];
extern const u8 gText_Pokegear_DesignHint[];
extern void m4aSongNumStop(u16 n);

//Time strings
extern const u8 gText_Sun[];
extern const u8 gText_Mon[];
extern const u8 gText_Tue[];
extern const u8 gText_Wed[];
extern const u8 gText_Thu[];
extern const u8 gText_Fri[];
extern const u8 gText_Sat[]; 
extern const u8 gText_AM[]; 
extern const u8 gText_PM[]; 
static const struct BgTemplate sPokegearBgTemplates[] =
{
	[BG_TEXT] =
	{
		.bg = BG_TEXT,
		.charBaseIndex = 0,
		.mapBaseIndex = 31,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 0,
		.baseTile = 0,
	},
	[BG_1] =
	{
		.bg = BG_1,
		.charBaseIndex = 1,
		.mapBaseIndex = 30,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 1,
		.baseTile = 0,
	},
	[BG_2] =
	{
		.bg = BG_2,
		.charBaseIndex = 2,
		.mapBaseIndex = 29,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 2,
		.baseTile = 0,
	},
	[BG_BACKGROUND] =
	{
		.bg = BG_BACKGROUND,
		.charBaseIndex = 3,
		.mapBaseIndex = 28,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 3,
		.baseTile = 0,
	}, 
}; 


// Text fonts
static const struct TextColor sWhiteText =
{
	.bgColor = TEXT_COLOR_TRANSPARENT,
	.fgColor = TEXT_COLOR_WHITE,
	.shadowColor = TEXT_COLOR_DARK_GRAY,
}; 




#endif // GUARD_Pokegear_H