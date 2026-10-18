#include "../include/global.h" 
#include "Pokegear.h" 
#include "../include/bg.h"
#include "../include/data.h"
#include "../include/decompress.h"
#include "../include/event_data.h"
#include "../include/gpu_regs.h"
#include "../include/graphics.h"
#include "../include/item.h"
#include "../include/list_menu.h"
#include "../include/international_string_util.h"
#include "../include/main.h"
#include "../include/malloc.h"
#include "../include/menu.h"
#include "../include/menu_helpers.h" 
#include "../include/palette.h"
#include "../include/party_menu.h"
#include "../include/scanline_effect.h"
#include "../include/script.h"
#include "../include/sound.h"
#include "../include/string_util.h"
#include "../include/strings.h"
#include "../include/pokedex.h"
#include "../include/task.h"
#include "../include/text_window.h"
#include "../include/event_data.h"
#include "../include/overworld.h"
#include "../include/constants/items.h"
#include "../include/constants/field_weather.h"
#include "../include/constants/songs.h"
#include "../include/constants/event_objects.h"
#include "../include/constants/flags.h"
#include "../include/gba/types.h"
#include "../include/new/Vanilla_functions.h"

#include "PokegearSkins.h"
#include "PokegearUI.h"

#define cpos sPokegearPtr->cursorpos[0]   // selected option (0..3)
extern void QuestLog_CutRecording(void);
extern void InitRegionMapWithExitCB(u8 type, void (*callback)(void));


enum WindowIds
{
    WIN_BANNER,     // text on the banner plate: clock + name of the selected option
    WIN_HINT,       // "L/R: Design" next to the design dots
    WIN_COUNT
};

// banner plate in PokegearBg.png: x 68..177, y 36..66
#define PG_BANNER_W   104   // window width in pixels (13 tiles)
#define PG_BANNER_Y1  6     // clock line, relative to the window (window starts at y = 32)
#define PG_BANNER_Y2  19    // option name line

#define PHONECARD_MAX_CONTACTS 64 // size of sPhoneContacts[] must not exceed this
#define RADIO_MAX_STATIONS 16     // size of sRadioStations[] must not exceed this
#define RADIO_MAX_LINES    16     // lines per station
struct PokegearResources
{
  u8 cursorpos[2];
  u8* sBgTilemapBuffer;
  u8 ArrowSpriteIds[2];

  // Main screen state (RAM - see the note below about statics)
  u8 pgDesign;                  // design whose colours are shown (while fading: the one we left)
  u8 pgTarget;                  // chosen design
  u8 pgFadeStep;                // 0 = no colour fade running
  u8 pgFromCard;                // returned from a card: keep the design that was chosen
  u8 pgUnlocked;                // bit i set = option i can be selected
  u8 pgLastMinute;              // banner is redrawn when this or the cursor changes
  u8 pgLastCursor;
  u8 pgGlow;                    // sprite ids
  u8 pgIcon[POKEGEAR_NUM_OPTIONS];
  u8 pgDot[POKEGEAR_NUM_DESIGNS];
  u16 pgFromPal[16];            // colours at the start of the fade
  u16 pgCurPal[16];             // colours on screen

  // Phone card state. Must live here (RAM): linker.ld puts every global/static
  // in ROM, so a writable "static" variable can never change.
  u8 phoneScroll;
  u8 phoneCursor;
  u8 phoneMsgActive;
  u8 phonePortraitSpriteId;
  u16 phonePortraitIdx;
  struct SpriteTemplate phonePortraitTemplate; // the game keeps a pointer to it while the sprite exists
  u8 phoneVisibleCount;                        // how many contacts are registered right now
  u8 phoneVisible[PHONECARD_MAX_CONTACTS];     // list position -> index into sPhoneContacts

  // Radio card state (RAM - see the note above about statics)
  u16 radioFreq;                               // tuned frequency, tenths of MHz
  u16 radioSong;                               // song currently started by the radio (0 = none)
  u16 radioTimer;                              // frames the current line has been fully shown
  u8 radioState;                               // list position of the tuned station, or RADIO_STATE_*
  u8 radioOn;
  u8 radioMusicChanged;                        // the radio has taken over the background music
  u8 radioHold;                                // frames the D-pad has been held (tuning speed-up)
  u8 radioLine;                                // current line (position in radioLines)
  u8 radioLineCount;
  u8 radioLines[RADIO_MAX_LINES];              // indices into the tuned station's lines
  u8 radioAvailCount;
  u8 radioAvail[RADIO_MAX_STATIONS];           // receivable stations, sorted by frequency
  u8 radioNeedleSprite;
  u8 radioMarkerSprites[RADIO_MAX_STATIONS];
};   

#define sPokegearPtr (*((struct PokegearResources**) 0x203E038))  

extern u16 StdTextPal[];
static const struct WindowTemplate sPokegearWindowTemplates[] = 
{
    [WIN_BANNER] = 
    {
        .bg = BG_TEXT,
        .tilemapLeft = 9,   // x = 72
        .tilemapTop = 4,    // y = 32
        .width = 13,
        .height = 5,
        .paletteNum = 14,
        .baseBlock = 1,     // 65 tiles
    },
    [WIN_HINT] = 
    {
        .bg = BG_TEXT,
        .tilemapLeft = 1,   // x = 8
        .tilemapTop = 3,    // y = 24
        .width = 9,
        .height = 2,
        .paletteNum = 14,
        .baseBlock = 66,    // 18 tiles
    },
    DUMMY_WIN_TEMPLATE,
}; 

#define FLAG_MAP_CARD 0x200


//This file's functions
void CB2_ReturnToFieldWithOpenMenu(void); 
static void ClearTasksAndGraphicalStructs(void); 
static void ClearVramOamPlttRegs(void);
static void VBlankCB_Pokegear(void); 
static void MainCB2_Pokegear(void);
static void ReturnToFieldFromPokegear(void);
static bool8 InitPokegearGUI(void); 
static void Task_PokegearFadeIn(u8 taskId); 
static void Task_PokegearWaitForKeyPress(u8 taskId); 
static void LoadPokegearBgGfx(void);
static void SetUpPokegear_NormalField(void); 
static void Task_PokegearFadeOut(u8 taskId); 
static void FreeAndClosePokegear(u8 taskId); 
static void CleanWindow(u8 windowId); 
static void CommitWindow(u8 windowId); 
static void CleanWindows(void); 
static void CommitWindows(void); 
void Pokegear_Init(void); 
static void Task_RunPokegearOptionFuncOrScript(u8 taskId);
static void PrintPokegearBanner(void);
static void ShowTownMap(void);
static void ShowPhoneCard(void);
static void ShowRadioCard(void);
static void CB2_PhoneCard(void);
static void InitPhoneCardGUI(void);
static void LoadPhoneCardBgGfx(void);
static void Task_PhoneCardFadeIn(u8 taskId);
static void Task_PhoneCardInputHandler(u8 taskId);
static void Task_PhoneCardFadeOutToPokegear(u8 taskId);
static void PrintPhoneCardUI(void);
static void CreatePhoneCardPortrait(u16 graphicsId);
static void DestroyPhoneCardPortrait(void);

#define PHONECARD_MAX_VISIBLE 6

// One plate per option, in this order. `flag`: option is usable once this flag is set (0 = always).
// A locked option stays on its plate, greyed out, and the cursor skips it.
// The flags are set in PokegearSkins.h (POKEGEAR_FLAG_MAP / _RADIO / _PHONE).
static const struct PokegearOption sPokegearOptionsTable[POKEGEAR_NUM_OPTIONS] = 
{
  [Pokegear_MAPCARD] =
  {
    .id = Pokegear_MAPCARD,
    .text = (u8*) gText_Pokegear_MapCard,
    .flag = POKEGEAR_FLAG_MAP,
    .script = NULL,
    .func = ShowTownMap
  },
  [Pokegear_RADIOCARD] =
  {
    .id = Pokegear_RADIOCARD,
    .text = (u8*) gText_Pokegear_RadioCard,
    .flag = POKEGEAR_FLAG_RADIO,
    .script = NULL,
    .func = ShowRadioCard
  },
  [Pokegear_PHONECARD] =
  {
    .id = Pokegear_PHONECARD,
    .text = (u8*) gText_Pokegear_PhoneCard,
    .flag = POKEGEAR_FLAG_PHONE,
    .script = NULL,
    .func = ShowPhoneCard
  },
  [Pokegear_EXIT] =
  {
    .id = Pokegear_EXIT,
    .text = (u8*) gText_Pokegear_Exit,
    .flag = 0,
    .script = NULL,
    .func = NULL            // closes the Pokegear
  },
}; 

 enum {
  REGIONMAP_TYPE_NORMAL,
  REGIONMAP_TYPE_WALL,
  REGIONMAP_TYPE_FLY,
  REGIONMAP_TYPE_COUNT
};

 static void ShowTownMap(void) {
  QuestLog_CutRecording();
  InitRegionMapWithExitCB(1, CB2_ReturnToFieldWithOpenMenu);
}

static void ClearTasksAndGraphicalStructs(void)
{
	ScanlineEffect_Stop();
	ResetTasks();
	ResetSpriteData();
	ResetTempTileDataBuffers();
	ResetPaletteFade();
	FreeAllSpritePalettes();
} 
 
static void ClearVramOamPlttRegs(void)
{
	DmaFill16(3, 0, VRAM, VRAM_SIZE);
	DmaFill32(3, 0, OAM, OAM_SIZE);
	DmaFill16(3, 0, PLTT, PLTT_SIZE);SetGpuReg(REG_OFFSET_DISPCNT,  0);
    SetGpuReg(REG_OFFSET_BG0CNT,   0);
    SetGpuReg(REG_OFFSET_BG0HOFS,  0);
    SetGpuReg(REG_OFFSET_BG0VOFS,  0);
    SetGpuReg(REG_OFFSET_BG1CNT,   0);
    SetGpuReg(REG_OFFSET_BG1HOFS,  0);
    SetGpuReg(REG_OFFSET_BG1VOFS,  0);
    SetGpuReg(REG_OFFSET_BG2CNT,   0);
    SetGpuReg(REG_OFFSET_BG2HOFS,  0);
    SetGpuReg(REG_OFFSET_BG2VOFS,  0);
    SetGpuReg(REG_OFFSET_BG3CNT,   0);
    SetGpuReg(REG_OFFSET_BG3HOFS,  0);
    SetGpuReg(REG_OFFSET_BG3VOFS,  0);
    SetGpuReg(REG_OFFSET_WIN0H,    0);
    SetGpuReg(REG_OFFSET_WIN0V,    0);
    SetGpuReg(REG_OFFSET_WININ,    0);
    SetGpuReg(REG_OFFSET_WINOUT,   0);
    SetGpuReg(REG_OFFSET_BLDCNT,   0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY,     0);
	
}

static void VBlankCB_Pokegear(void)
{
	LoadOam();
	ProcessSpriteCopyRequests();
	TransferPlttBuffer();
}
static void MainCB2_Pokegear(void)
{
	RunTasks();
  AnimateSprites();
	BuildOamBuffer();
	UpdatePaletteFade();
}

void CB2_Pokegear(void)
{
	switch (gMain.state) {
		case 0:
		default:
			SetBGMVolume_SuppressHelpSystemReduction(160);
			SetVBlankCallback(NULL);
			ClearVramOamPlttRegs();
			SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_ON);
			gMain.state++;
			break;
		case 1:
			ClearTasksAndGraphicalStructs();
			gMain.state++;
			break;
		case 2: 
		  sPokegearPtr->sBgTilemapBuffer = Malloc(0x1000);
			ResetBgsAndClearDma3BusyFlags(0);
			InitBgsFromTemplates(0, sPokegearBgTemplates, NELEMS(sPokegearBgTemplates));
 			SetBgTilemapBuffer(BG_BACKGROUND, sPokegearPtr->sBgTilemapBuffer);
			gMain.state++;
			break;
		case 3: 
		  if (!(cpos))
		     cpos = 0;
		  SetUpPokegear_NormalField();
			LoadPokegearBgGfx(); 
			gMain.state++;
			break;
		case 4:
			if (!free_temp_tile_data_buffers_if_possible())
			{
			  ShowBg(BG_TEXT);
				ShowBg(BG_BACKGROUND);
				CopyBgTilemapBufferToVram(BG_BACKGROUND);
				gMain.state++;
			}
			break;
		case 5:
	    Free(sPokegearPtr->sBgTilemapBuffer);
			InitWindows(sPokegearWindowTemplates);
			DeactivateAllTextPrinters();
			gMain.state++;
			break;
		case 6:
			BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
			gMain.state++;
			break;
		case 7:
			SetVBlankCallback(VBlankCB_Pokegear);
			InitPokegearGUI();
			CreateTask(Task_PokegearFadeIn, 0);
			SetMainCallback2(MainCB2_Pokegear);
			gMain.state = 0;
			break;
	}
} 

static void Task_PokegearFadeIn(u8 taskId)
{
	if (!gPaletteFade.active)
		gTasks[taskId].func = Task_PokegearWaitForKeyPress;
}  

static void LoadPokegearBgGfx(void)
{
	DecompressAndCopyTileDataToVram(BG_BACKGROUND, PokegearBgTiles, 0, 0, 0);
	LZDecompressWram(PokegearBgMap, sPokegearPtr->sBgTilemapBuffer);
	LoadPalette(sPokegearSkinPal[sPokegearPtr->pgDesign], 0, 0x20);   // colours of the chosen design
	LoadPalette(StdTextPal, 14*16, 0x20);
} 

// ---------------------------------------------------------------------------
// Designs: the colours of the background are changed with palette updates
// ---------------------------------------------------------------------------

// mix two RGB555 colours, t = 0 (all `from`) .. 16 (all `to`)
static u16 PgBlendColor(u16 from, u16 to, u8 t)
{
	u16 r = ((from & 0x1F) * (16 - t) + (to & 0x1F) * t) >> 4;
	u16 g = (((from >> 5) & 0x1F) * (16 - t) + ((to >> 5) & 0x1F) * t) >> 4;
	u16 b = (((from >> 10) & 0x1F) * (16 - t) + ((to >> 10) & 0x1F) * t) >> 4;

	return r | (g << 5) | (b << 10);
}

// The design chosen on the main screen. The phone card and the radio card use the same colours.
// (the main screen writes the choice to special var 0x8002 when a card is opened)
static u8 PokegearGetDesign(void)
{
	u16 design;

#if POKEGEAR_DESIGN_VAR
	design = VarGet(POKEGEAR_DESIGN_VAR);
#else
	design = VarGet(0x8002);
#endif
	return (design < POKEGEAR_NUM_DESIGNS) ? design : 0;
}

// start a smooth colour change towards a design (also works in the middle of another one)
static void PokegearStartDesignFade(u8 target)
{
	for (u8 i = 0; i < 16; i++)
		sPokegearPtr->pgFromPal[i] = sPokegearPtr->pgCurPal[i];
	sPokegearPtr->pgTarget = target;
	sPokegearPtr->pgFadeStep = 1;
#if POKEGEAR_DESIGN_VAR
	VarSet(POKEGEAR_DESIGN_VAR, target);
#endif
}

static void PokegearUpdateDesignFade(void)
{
	u8 step = sPokegearPtr->pgFadeStep;
	u8 t;

	if (step == 0)
		return;
	t = (step >= POKEGEAR_DESIGN_FADE_FRAMES) ? 16 : (step * 16) / POKEGEAR_DESIGN_FADE_FRAMES;
	for (u8 i = 0; i < 16; i++)
		sPokegearPtr->pgCurPal[i] = PgBlendColor(sPokegearPtr->pgFromPal[i], sPokegearSkinPal[sPokegearPtr->pgTarget][i], t);
	LoadPalette(sPokegearPtr->pgCurPal, 0, 0x20);
	if (step >= POKEGEAR_DESIGN_FADE_FRAMES)
	{
		sPokegearPtr->pgFadeStep = 0;
		sPokegearPtr->pgDesign = sPokegearPtr->pgTarget;
	}
	else
	{
		sPokegearPtr->pgFadeStep = step + 1;
	}
}

// grey copy of the icon palette (locked options)
static void PgBuildLockedPalette(u16 *dst)
{
	for (u8 i = 0; i < 16; i++)
	{
		u16 c = pg_mapPal[i];
		u16 l = ((((c & 0x1F) + ((c >> 5) & 0x1F) + ((c >> 10) & 0x1F)) / 3) * 2) / 3;

		dst[i] = l | (l << 5) | (l << 10);
	}
}

// ---------------------------------------------------------------------------
// Sprite callbacks
// ---------------------------------------------------------------------------

// Affine matrix of a sprite. `size` is the size it is DRAWN at: 0x100 = normal, 0x200 = twice as big.
// (The hardware matrix holds the inverse - a bigger number there would mean a SMALLER sprite -
//  so the size is converted here and everything else just talks in sizes.)
static void PgSetSpriteSize(struct Sprite *sprite, u16 size)
{
	s16 m = 0x10000 / size;

	SetOamMatrixRotationScaling(sprite->oam.matrixNum, m, m, 0);
}

// give a sprite its own matrix and take it away from the animation engine
static void PgInitAffineSprite(u8 spriteId, u16 size)
{
	InitSpriteAffineAnim(&gSprites[spriteId]);
	gSprites[spriteId].affineAnimPaused = TRUE;     // we write the matrix ourselves
	PgSetSpriteSize(&gSprites[spriteId], size);
}

static const u16 sPgIntroSize[] = {0x60, 0x80, 0xA0, 0xC0, 0xE0, 0xF4, 0x108, 0x110, 0x108, 0x100};  // "pop"
static const u16 sPgIconPulse[16] = {0x100, 0x104, 0x108, 0x10C, 0x110, 0x114, 0x118, 0x118, 0x118, 0x114, 0x110, 0x10C, 0x108, 0x104, 0x100, 0x100};
static const u16 sPgDotPulse[16]  = {0x150, 0x154, 0x158, 0x15C, 0x160, 0x164, 0x168, 0x168, 0x168, 0x164, 0x160, 0x15C, 0x158, 0x154, 0x150, 0x150};

// Option icon: pops in one after the other, then the selected one shows its bright frame and "breathes".
//   data[0] = option number   data[1] = age (negative = waiting to pop in)
//   data[2] = last selected state   data[3] = pulse counter
void PokegearIconCallback(struct Sprite *sprite) 
{ 
	u8 selected = (sprite->data[0] == cpos);

	if (sprite->data[1] < 0)                        // waiting for its turn
	{
		sprite->data[1]++;
		if (sprite->data[1] == 0)
			sprite->invisible = FALSE;
		return;
	}
	if (sprite->data[1] < (s16) ARRAY_COUNT(sPgIntroSize))     // pop-in
	{
		PgSetSpriteSize(sprite, sPgIntroSize[sprite->data[1]]);
		sprite->data[1]++;
		sprite->data[2] = 0xFF;                     // force an update when the pop-in is over
		return;
	}
	if (sprite->data[2] != selected)
	{
		sprite->data[2] = selected;
		sprite->data[3] = 0;
		StartSpriteAnim(sprite, selected);          // frame 0 = normal, frame 1 = selected
		if (!selected)
			PgSetSpriteSize(sprite, 0x100);
	}
	if (selected)
		PgSetSpriteSize(sprite, sPgIconPulse[(sprite->data[3]++ >> 1) & 15]);
}

// Glow under the selected plate: slides to the new plate and pulses.
// The sprite is drawn in "blend" mode, so the hardware mixes it with the background;
// the pulse is simply a changing BLDALPHA.
static const u8 sGlowAlpha[16] = {5, 6, 7, 8, 9, 10, 11, 11, 10, 9, 8, 7, 6, 5, 5, 5};

void PokegearGlowCallback(struct Sprite *sprite)
{
	s16 dx = sPgPlateX[cpos] - sprite->pos1.x;
	u8 eva;

	if (dx != 0)
	{
		s16 step = dx / 3;
		if (step == 0)
			step = (dx > 0) ? 1 : -1;
		sprite->pos1.x += step;
	}
	sprite->data[0]++;
	eva = sGlowAlpha[(sprite->data[0] >> 2) & 15];
	SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(eva, 16 - eva));
}

// Design dot: the chosen design's dot is drawn bigger and pulses.
//   data[0] = design number   data[2] = last state   data[3] = pulse counter
void PokegearDotCallback(struct Sprite *sprite)
{
	u8 chosen = (sprite->data[0] == sPokegearPtr->pgTarget);

	if (chosen)
	{
		PgSetSpriteSize(sprite, sPgDotPulse[(sprite->data[3]++ >> 1) & 15]);
	}
	else if (sprite->data[2] != chosen)
	{
		PgSetSpriteSize(sprite, 0x100);
	}
	sprite->data[2] = chosen;
}

// ---------------------------------------------------------------------------
// Main screen
// ---------------------------------------------------------------------------
static bool8 InitPokegearGUI(void)
{ 
	u16 pal[16];
	u8 id;

	CleanWindows();

	// Alpha blending: sprites in "blend" mode (the glow) are mixed with BG3 / the backdrop.
	SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG3 | BLDCNT_TGT2_BD);
	SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(8, 8));

	// sprite palettes: icons, greyed icons (locked options), glow, one per design dot
	LoadSpritePalette(&(struct SpritePalette){pg_mapPal, GFXTAG_PG_ICON_PAL});
	PgBuildLockedPalette(pal);
	LoadSpritePalette(&(struct SpritePalette){pal, GFXTAG_PG_ICON_LOCKED});
	LoadSpritePalette(&sPgGlowPalette);
	for (u8 i = 0; i < POKEGEAR_NUM_DESIGNS; i++)
	{
		for (u8 j = 0; j < 16; j++)
			pal[j] = 0;
		pal[1] = sPokegearSkinDot[i];
		pal[2] = PG_RGB(31, 31, 31);
		LoadSpritePalette(&(struct SpritePalette){pal, GFXTAG_PG_DOT_PAL0 + i});
	}

	// one icon per plate (affine sprites)
	for (u8 i = 0; i < POKEGEAR_NUM_OPTIONS; i++)
	{
		LoadSpriteSheet(&sPgIconSheets[i]);
		id = CreateSprite(&sPgIconTemplates[i], sPgPlateX[i], PG_PLATE_Y, 0);
		sPokegearPtr->pgIcon[i] = id;
		if (id >= MAX_SPRITES)
			continue;
		PgInitAffineSprite(id, sPgIntroSize[0]);
		gSprites[id].data[0] = i;
		gSprites[id].data[1] = -4 * i;              // icons pop in one after the other
		gSprites[id].data[2] = 0xFF;
		gSprites[id].data[3] = 0;
		if (i != 0)
			gSprites[id].invisible = TRUE;
		if (!(sPokegearPtr->pgUnlocked & (1 << i)))
			gSprites[id].oam.paletteNum = IndexOfSpritePaletteTag(GFXTAG_PG_ICON_LOCKED);
	}

	// glow under the selected plate
	LoadSpriteSheet(&sPgGlowSheet);
	sPokegearPtr->pgGlow = CreateSprite(&sPgGlowTemplate, sPgPlateX[cpos], PG_GLOW_Y, 1);

	// design dots (top left)
	LoadSpriteSheet(&sPgDotSheet);
	for (u8 i = 0; i < POKEGEAR_NUM_DESIGNS; i++)
	{
		id = CreateSprite(&sPgDotTemplates[i], PG_DOT_X0 + PG_DOT_DX * i, PG_DOT_Y, 0);
		sPokegearPtr->pgDot[i] = id;
		if (id >= MAX_SPRITES)
			continue;
		PgInitAffineSprite(id, 0x100);
		gSprites[id].data[0] = i;
		gSprites[id].data[2] = 0xFF;
		gSprites[id].data[3] = 0;
	}

	WindowPrint(WIN_HINT, 0, 2, 2, &sWhiteText, 0, gText_Pokegear_DesignHint);
	sPokegearPtr->pgLastMinute = 0xFF;      // forces the first draw of the banner
	PrintPokegearBanner();
	CommitWindows();
	return TRUE;
}

// move the cursor to the next option that is not locked (left: -1, right: +1)
static void PokegearMoveCursor(s8 dir)
{
	s8 pos = cpos;

	for (;;)
	{
		pos += dir;
		if (pos < 0 || pos >= POKEGEAR_NUM_OPTIONS)
			return;                         // nothing selectable in that direction
		if (sPokegearPtr->pgUnlocked & (1 << pos))
			break;
	}
	PlaySE(SE_SELECT);
	cpos = pos;
}

static void Task_PokegearWaitForKeyPress(u8 taskId)
{ 
	PokegearUpdateDesignFade();
	if (gClock.minute != sPokegearPtr->pgLastMinute || cpos != sPokegearPtr->pgLastCursor)
		PrintPokegearBanner();

	if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && cpos == Pokegear_EXIT)) 
	{ 
		PlaySE(SE_PC_OFF);
		BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
		gTasks[taskId].func = Task_PokegearFadeOut;
	} 
	else if (JOY_NEW(A_BUTTON))
	{
		PlaySE(SE_SELECT);
		VarSet(0x8000, cpos);                           // so the cursor and the design are
		VarSet(0x8002, sPokegearPtr->pgTarget);         // restored when we come back from the card
		BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
		gTasks[taskId].func = Task_RunPokegearOptionFuncOrScript;
	}
	else if (JOY_NEW(DPAD_RIGHT)) 
	{
		PokegearMoveCursor(1);
	} 
	else if (JOY_NEW(DPAD_LEFT))
	{
		PokegearMoveCursor(-1);
	} 
	else if (JOY_NEW(L_BUTTON))
	{
		PlaySE(SE_SELECT);
		PokegearStartDesignFade((sPokegearPtr->pgTarget + POKEGEAR_NUM_DESIGNS - 1) % POKEGEAR_NUM_DESIGNS);
	}
	else if (JOY_NEW(R_BUTTON))
	{
		PlaySE(SE_SELECT);
		PokegearStartDesignFade((sPokegearPtr->pgTarget + 1) % POKEGEAR_NUM_DESIGNS);
	}
} 

static void Task_RunPokegearOptionFuncOrScript(u8 taskId)
{
	if (!gPaletteFade.active)
	{ 
		const struct PokegearOption *option = &sPokegearOptionsTable[cpos];

		if (option->func != NULL) 
			SetMainCallback2(option->func);
		else
		{
			ScriptContext1_SetupScript(option->script); 
			SetMainCallback2(ReturnToFieldFromPokegear);
		}
		FreeAndClosePokegear(taskId);
	}
} 

void Pokegear_Init(void)
{
	if (!gPaletteFade.active)
	{
		ResetBgsAndClearDma3BusyFlags(0);
		gHelpSystemEnabled = FALSE;
		CleanupOverworldWindowsAndTilemaps();
		sPokegearPtr = Calloc(sizeof(struct PokegearResources));
		PlaySE(SE_PC_LOGIN);
		SetMainCallback2(CB2_Pokegear);
	} 
}

// which options can be used, where the cursor starts and which design is shown
static void SetUpPokegear_NormalField(void) 
{
	u8 mask = 0;
	u8 design;

	for (u8 i = 0; i < POKEGEAR_NUM_OPTIONS; i++)
	{ 
		u16 flag = sPokegearOptionsTable[i].flag;

		if (flag == 0 || FlagGet(flag))
			mask |= 1 << i;
	} 
	sPokegearPtr->pgUnlocked = mask;

	if (cpos >= POKEGEAR_NUM_OPTIONS || !(mask & (1 << cpos)))
	{
		cpos = 0;
		while (cpos < POKEGEAR_NUM_OPTIONS - 1 && !(mask & (1 << cpos)))
			cpos++;
	}

	if (sPokegearPtr->pgFromCard)
	{
		design = sPokegearPtr->pgDesign;            // coming back from a card
	}
	else
	{
		design = 0;
#if POKEGEAR_DESIGN_VAR
		design = VarGet(POKEGEAR_DESIGN_VAR);
#endif
	}
	if (design >= POKEGEAR_NUM_DESIGNS)
		design = 0;
	sPokegearPtr->pgDesign = design;
	sPokegearPtr->pgTarget = design;
	sPokegearPtr->pgFadeStep = 0;
	for (u8 i = 0; i < 16; i++)
		sPokegearPtr->pgCurPal[i] = sPokegearSkinPal[design][i];
}

static void FreeAndClosePokegear(u8 taskId)
{
	SetGpuReg(REG_OFFSET_BLDCNT, 0);        // alpha blending off again
	SetGpuReg(REG_OFFSET_BLDALPHA, 0);
	Free(sPokegearPtr);
	FreeAllWindowBuffers();
	BGMVolumeMax_EnableHelpSystemReduction();
	DestroyTask(taskId);
} 

static void Task_PokegearFadeOut(u8 taskId)
{
	if (!gPaletteFade.active)
	{
	  gHelpSystemEnabled = TRUE;
		SetMainCallback2(ReturnToFieldFromPokegear);
		FreeAndClosePokegear(taskId); 
	}
}
static void ReturnToFieldFromPokegear(void) {
  FieldClearVBlankHBlankCallbacks();
  gFieldCallback = FieldCB_WarpExitFadeFromBlack;
  CB2_ReturnToField();
}

static void CleanWindow(u8 windowId)
{
	FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
}

static void CleanWindows(void)
{
	for (u32 i = 0; i < WIN_COUNT; ++i)
		CleanWindow(i);
}

static void CommitWindow(u8 windowId)
{
	CopyWindowToVram(windowId,COPYWIN_BOTH);
	PutWindowTilemap(windowId);
}

static void CommitWindows(void)
{
	for (u32 i = 0; i < WIN_COUNT; ++i)
		CommitWindow(i);
}

void CB2_ReturnToFieldWithOpenMenu(void)
{ 
	sPokegearPtr = Calloc(sizeof(struct PokegearResources));
	cpos = VarGet(0x8000); 
	sPokegearPtr->pgDesign = VarGet(0x8002);        // design that was chosen before opening the card
	sPokegearPtr->pgFromCard = TRUE;
	SetMainCallback2(CB2_Pokegear);
} 

extern const u8 gText_PhoneCard_HintList[];
extern const u8 gText_PhoneCard_Idle[];
extern const u8 gText_PhoneCard_NoReply[];
extern const u8 gText_PhoneCard_NoContacts[];
extern const u8 gText_PhoneCard_NoContactsBox[];

// The contact table (names, portraits, registered flags, messages by flag) lives in its own file
#include "PhoneContacts.h"
#include "RadioStations.h"

_Static_assert(ARRAY_COUNT(sPhoneContacts) <= PHONECARD_MAX_CONTACTS, "too many phone contacts");
_Static_assert(ARRAY_COUNT(sRadioStations) <= RADIO_MAX_STATIONS, "too many radio stations");

// Number of contacts that are currently registered (the list the player sees)
#define PHONECARD_ENTRY_COUNT ((int)sPokegearPtr->phoneVisibleCount)

// 1 = portrait comes from the overworld sprite table, 0 = no portrait (handy to bisect a crash)
#define PHONECARD_OW_PORTRAIT  1

// Layout (pixels). Matches graphics/PhoneCardBg.png (HGSS-style design)
#define PHONECARD_PORTRAIT_X   22   // centre of the portrait column in the top box
#define PHONECARD_PORTRAIT_Y   28
#define PHONECARD_ROW_HEIGHT   14   // 6 rows fill the list panel
#define PHONECARD_CURSOR_X     2    // arrow glyph, relative to WIN_PH_LIST (window starts at x = 8)
#define PHONECARD_NAME_X       14   // contact name, relative to WIN_PH_LIST
#define PHONECARD_ROLE_X       104  // role column, relative to WIN_PH_LIST
#define PHONECARD_BOX_X        2    // name + message text, relative to WIN_PH_BOX (window starts at x = 40)
#define PHONECARD_BOX_NAME_Y   4
#define PHONECARD_BOX_MSG_Y    20
#define PHONECARD_ARROW_X      224  // scroll arrows sit on the two buttons on the right
#define PHONECARD_ARROW_UP_Y   76
#define PHONECARD_ARROW_DOWN_Y 120

// The phone card has its own window set (the main Pokegear screen keeps sMenuWindowTemplates)
enum PhoneWindows
{
    WIN_PH_BOX,     // top box: contact name + message (typewriter)
    WIN_PH_LIST,    // names, cursor, roles
    WIN_PH_HINT,    // text on the bottom button
    WIN_PH_COUNT
};

static const struct WindowTemplate sPhoneWindowTemplates[] =
{
    [WIN_PH_BOX] =
    {
        .bg = BG_TEXT, .tilemapLeft = 5, .tilemapTop = 0, .width = 24, .height = 7,
        .paletteNum = 14, .baseBlock = 1,       // 168 tiles
    },
    [WIN_PH_LIST] =
    {
        .bg = BG_TEXT, .tilemapLeft = 1, .tilemapTop = 7, .width = 25, .height = 11,
        .paletteNum = 14, .baseBlock = 169,     // 275 tiles
    },
    [WIN_PH_HINT] =
    {
        .bg = BG_TEXT, .tilemapLeft = 5, .tilemapTop = 18, .width = 20, .height = 2,
        .paletteNum = 14, .baseBlock = 444,     // 40 tiles
    },
    DUMMY_WIN_TEMPLATE,
};

#define PHONECARD_TEXT_SPEED   1    // 1 = fast typewriter, 0 = instant, bigger = slower

#define IS_ROM_PTR(p)          (((u32)(p) >> 25) == 4) // 0x08000000 - 0x09FFFFFF

// Mutable phone card state (all in the RAM struct, see struct PokegearResources)
#define sPhoneCardScroll            (sPokegearPtr->phoneScroll)
#define sPhoneCardCursor            (sPokegearPtr->phoneCursor)   // 0 .. PHONECARD_MAX_VISIBLE-1
#define sPhoneCardMsgActive         (sPokegearPtr->phoneMsgActive)
#define sPhoneCardPortraitSpriteId  (sPokegearPtr->phonePortraitSpriteId)
#define sPhoneCardPortraitIdx       (sPokegearPtr->phonePortraitIdx)
#define sPhonePortraitTemplate      (sPokegearPtr->phonePortraitTemplate)

// The panels in PhoneCardBg are white, so the text has to be dark.
static const struct TextColor sPhoneDarkText =
{
    .bgColor = TEXT_COLOR_TRANSPARENT,
    .fgColor = TEXT_COLOR_DARK_GRAY,
    .shadowColor = TEXT_COLOR_LIGHT_GRAY,
};

// Role column (right half of the list)
static const struct TextColor sPhoneRoleText =
{
    .bgColor = TEXT_COLOR_TRANSPARENT,
    .fgColor = TEXT_COLOR_LIGHT_GRAY,
    .shadowColor = TEXT_COLOR_DARK_GRAY,
};

static const u8 sText_PhoneCursor[] = {0xEF, 0xFF}; // right-pointing arrow + end of string

// Highlighted contact
static const struct TextColor sPhoneSelText =
{
    .bgColor = TEXT_COLOR_TRANSPARENT,
    .fgColor = TEXT_COLOR_RED,
    .shadowColor = TEXT_COLOR_LIGHT_RED,
};


// Main Pokegear screen: nothing to scroll here, so the bar stays hidden
// (the phone card has its own scrollbar below).
void ScrollBarCallback(struct Sprite *sprite)
{
    sprite->invisible = TRUE;
}

const u8 * sDayNames[] =
{
  gText_Sun,
  gText_Mon,
  gText_Tue,
  gText_Wed,
  gText_Thu,
  gText_Fri,
  gText_Sat,
};

extern u8 gText_Pokegear_TimeBase_12Hr[];

// Banner plate: clock on the first line, name of the selected option on the second (both centred)
static void PrintPokegearBanner(void)
{
	const u8* amPMString = (gClock.hour >= 12) ? gText_PM : gText_AM;
	const u8* name = sPokegearOptionsTable[cpos].text;
	s32 w1, w2, x;

	ConvertIntToDecimalStringN(gStringVar1, (gClock.hour == 0) ? 12 : (gClock.hour > 12) ? gClock.hour - 12 : gClock.hour, STR_CONV_MODE_RIGHT_ALIGN, 2); //Hour - 12hr format
	ConvertIntToDecimalStringN(gStringVar2, gClock.minute, STR_CONV_MODE_LEADING_ZEROS, 2); //Minute
	StringCopy(gStringVar3, sDayNames[gClock.dayOfWeek]); //Day of Week
	StringExpandPlaceholders(gStringVar4, gText_Pokegear_TimeBase_12Hr);

	CleanWindow(WIN_BANNER);
	w1 = GetStringWidth(0, gStringVar4, 0);
	w2 = GetStringWidth(0, amPMString, 0);
	x = (PG_BANNER_W - (w1 + w2)) / 2;
	WindowPrint(WIN_BANNER, 0, x, PG_BANNER_Y1, &sWhiteText, 0, gStringVar4);
	WindowPrint(WIN_BANNER, 0, x + w1, PG_BANNER_Y1, &sWhiteText, 0, amPMString);
	x = (PG_BANNER_W - GetStringWidth(0, name, 0)) / 2;
	WindowPrint(WIN_BANNER, 0, x, PG_BANNER_Y2, &sWhiteText, 0, name);
	CommitWindow(WIN_BANNER);

	sPokegearPtr->pgLastMinute = gClock.minute;
	sPokegearPtr->pgLastCursor = cpos;
}
static void ShowPhoneCard(void)
{
    QuestLog_CutRecording();
    // The Pokegear freed its resources before running this option, so make new ones.
    sPokegearPtr = Calloc(sizeof(struct PokegearResources));
    gMain.state = 0;
    SetMainCallback2(CB2_PhoneCard);
}

static void CB2_PhoneCard(void)
{
    switch (gMain.state)
    {
        case 0:
        default:
            SetVBlankCallback(NULL);
            ClearVramOamPlttRegs();
            SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_ON);
            gMain.state++;
            break;
        case 1:
            ClearTasksAndGraphicalStructs();
            gMain.state++;
            break;
        case 2:
            sPokegearPtr->sBgTilemapBuffer = Malloc(0x1000);
            ResetBgsAndClearDma3BusyFlags(0);
            InitBgsFromTemplates(0, sPokegearBgTemplates, NELEMS(sPokegearBgTemplates));
            SetBgTilemapBuffer(BG_BACKGROUND, sPokegearPtr->sBgTilemapBuffer);
            gMain.state++;
            break;
        case 3:
            LoadPhoneCardBgGfx();
            gMain.state++;
            break;
        case 4:
            if (!free_temp_tile_data_buffers_if_possible())
            {
                ShowBg(BG_TEXT);
                ShowBg(BG_BACKGROUND);
                CopyBgTilemapBufferToVram(BG_BACKGROUND);
                gMain.state++;
            }
            break;
        case 5:
            Free(sPokegearPtr->sBgTilemapBuffer);
            InitWindows(sPhoneWindowTemplates);
            DeactivateAllTextPrinters();
            gMain.state++;
            break;
        case 6:
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
            gMain.state++;
            break;
        case 7:
            SetVBlankCallback(VBlankCB_Pokegear);
            InitPhoneCardGUI();
            CreateTask(Task_PhoneCardFadeIn, 0);
            SetMainCallback2(MainCB2_Pokegear);
            gMain.state = 0;
            break;
    }
}

static void LoadPhoneCardBgGfx(void)
{
    DecompressAndCopyTileDataToVram(BG_BACKGROUND, PhoneCardBgTiles, 0, 0, 0);
    LZDecompressWram(PhoneCardBgMap, sPokegearPtr->sBgTilemapBuffer);
    LoadPalette(sPhoneCardSkinPal[PokegearGetDesign()], 0, 0x20);   // colours of the chosen design
    LoadPalette(StdTextPal, 14 * 16, 0x20); // text palette used by every window
}

// ---------------------------------------------------------------------------
// Portrait (overworld sprite table)
// ---------------------------------------------------------------------------
static const union AnimCmd sAnim_PhoneStill[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sAnimTable_PhoneStill[] =
{
    sAnim_PhoneStill
};

static void DestroyPhoneCardPortrait(void)
{
    if (sPhoneCardPortraitSpriteId < MAX_SPRITES)
    {
        DestroySpriteAndFreeResources(&gSprites[sPhoneCardPortraitSpriteId]);
        sPhoneCardPortraitSpriteId = MAX_SPRITES;
    }
}

// Reads the contact's entry from the overworld sprite table and shows its
// "standing, facing down" frame. Only table data is used - no per-contact PNG.
static void CreatePhoneCardPortrait(u16 graphicsId)
{
    DestroyPhoneCardPortrait();

#if PHONECARD_OW_PORTRAIT
    {
        const struct EventObjectGraphicsInfo* info = GetEventObjectGraphicsInfoOriginal(graphicsId);
        struct SpriteSheet sheet;
        u8 paletteSlot, spriteId;

        // refuse anything that does not look like a real table entry
        if (!IS_ROM_PTR(info) || !IS_ROM_PTR(info->images) || !IS_ROM_PTR(info->oam))
            return;
        if (!IS_ROM_PTR(info->images[0].data) || info->images[0].size == 0)
            return;

        paletteSlot = AllocSpritePalette(info->paletteTag1);
        if (paletteSlot == 0xFF)
            return;
        PatchObjectPalette(info->paletteTag1, paletteSlot);

        sheet.data = info->images[0].data; // frame 0 = standing, facing down
        sheet.size = info->images[0].size;
        sheet.tag = GFXTAG_PHONE_PORTRAIT;
        LoadSpriteSheet(&sheet);

        sPhonePortraitTemplate.tileTag = GFXTAG_PHONE_PORTRAIT;
        sPhonePortraitTemplate.paletteTag = info->paletteTag1;
        sPhonePortraitTemplate.oam = info->oam; // 16x16 / 16x32 / 32x32 shape from the table
        sPhonePortraitTemplate.anims = sAnimTable_PhoneStill;
        sPhonePortraitTemplate.images = NULL;
        sPhonePortraitTemplate.affineAnims = gDummySpriteAffineAnimTable;
        sPhonePortraitTemplate.callback = SpriteCallbackDummy;

        spriteId = CreateSprite(&sPhonePortraitTemplate, PHONECARD_PORTRAIT_X, PHONECARD_PORTRAIT_Y, 0);
        if (spriteId >= MAX_SPRITES)
        {
            FreeSpriteTilesByTag(GFXTAG_PHONE_PORTRAIT);
            FreeSpritePaletteByTag(info->paletteTag1);
            return;
        }
        gSprites[spriteId].oam.priority = 1;
        sPhoneCardPortraitSpriteId = spriteId;
    }
#endif
}

static bool8 PhoneFlagCheck(u16 flag)
{
    if (flag == PHONE_ALWAYS)
        return TRUE;
    if (flag == PHONE_NEVER)
        return FALSE;
    return FlagGet(flag);
}

// Which contacts are registered? (checked each time the phone card opens)
static void BuildPhoneVisibleList(void)
{
    sPokegearPtr->phoneVisibleCount = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sPhoneContacts); ++i)
    {
        if (PhoneFlagCheck(sPhoneContacts[i].registeredFlag))
            sPokegearPtr->phoneVisible[sPokegearPtr->phoneVisibleCount++] = i;
    }
}

// contact at a given position of the visible list
static const struct PhoneContact* PhoneContactAt(u16 listPos)
{
    return &sPhoneContacts[sPokegearPtr->phoneVisible[listPos]];
}

// first message (top to bottom) whose flag is set
static const u8* GetPhoneMessage(const struct PhoneContact* contact)
{
    for (u8 i = 0; i < contact->messageCount; ++i)
    {
        if (PhoneFlagCheck(contact->messages[i].flag))
            return contact->messages[i].text;
    }
    return gText_PhoneCard_NoReply;
}

static void InitPhoneCardGUI(void)
{
    sPhoneCardScroll = 0;
    sPhoneCardCursor = 0;
    sPhoneCardMsgActive = FALSE;
    sPhoneCardPortraitSpriteId = MAX_SPRITES;
    sPhoneCardPortraitIdx = 0xFFFF;
    BuildPhoneVisibleList();

    for (u8 i = 0; i < WIN_PH_COUNT; ++i)
    {
        CleanWindow(i);
        PutWindowTilemap(i);
    }

    // "more above / more below" arrows, drawn on the two buttons on the right
    LoadSpriteSheet(&(struct SpriteSheet){arrow_upTiles, 16 * 16 / 2, GFXTAG_ARROW_UP});
    LoadSpritePalette(&(struct SpritePalette){arrow_upPal, GFXTAG_ARROW_UP});
    LoadSpriteSheet(&(struct SpriteSheet){arrow_downTiles, 16 * 16 / 2, GFXTAG_ARROW_DOWN});
    LoadSpritePalette(&(struct SpritePalette){arrow_downPal, GFXTAG_ARROW_DOWN});
    sPokegearPtr->ArrowSpriteIds[0] = CreateSprite(&sSpriteTemplate_ArrowUp, PHONECARD_ARROW_X, PHONECARD_ARROW_UP_Y, 0);
    sPokegearPtr->ArrowSpriteIds[1] = CreateSprite(&sSpriteTemplate_ArrowDown, PHONECARD_ARROW_X, PHONECARD_ARROW_DOWN_Y, 0);

    // bottom button text (static)
    WindowPrint(WIN_PH_HINT, 1, 4, 0, &sPhoneDarkText, 0, gText_PhoneCard_HintList);
    CommitWindow(WIN_PH_HINT);

    PrintPhoneCardUI();
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

// Top box: contact name on the strip, and either the "press A" hint or the message
static void PrintPhoneBox(bool8 showMessage)
{
    CleanWindow(WIN_PH_BOX);
    if (PHONECARD_ENTRY_COUNT == 0)
    {
        WindowPrint(WIN_PH_BOX, 1, PHONECARD_BOX_X, PHONECARD_BOX_MSG_Y, &sPhoneRoleText, 0, gText_PhoneCard_NoContactsBox);
    }
    else
    {
        const struct PhoneContact* contact = PhoneContactAt(sPhoneCardScroll + sPhoneCardCursor);

        WindowPrint(WIN_PH_BOX, 1, PHONECARD_BOX_X, PHONECARD_BOX_NAME_Y, &sWhiteText, 0, contact->name);
        if (showMessage)
            WindowPrint(WIN_PH_BOX, 1, PHONECARD_BOX_X, PHONECARD_BOX_MSG_Y, &sWhiteText, PHONECARD_TEXT_SPEED, GetPhoneMessage(contact));
        else
            WindowPrint(WIN_PH_BOX, 1, PHONECARD_BOX_X, PHONECARD_BOX_MSG_Y, &sPhoneRoleText, 0, gText_PhoneCard_Idle);
    }
    CommitWindow(WIN_PH_BOX);
}

static void PrintPhoneCardUI(void)
{
    u16 selected = sPhoneCardScroll + sPhoneCardCursor;

    CleanWindow(WIN_PH_LIST);

    if (PHONECARD_ENTRY_COUNT == 0)
    {
        // nobody registered yet
        WindowPrint(WIN_PH_LIST, 1, PHONECARD_NAME_X, 0, &sPhoneRoleText, 0, gText_PhoneCard_NoContacts);
        CommitWindow(WIN_PH_LIST);
        PrintPhoneBox(FALSE);
        DestroyPhoneCardPortrait();
        sPhoneCardPortraitIdx = 0xFFFF;
        gSprites[sPokegearPtr->ArrowSpriteIds[0]].invisible = TRUE;
        gSprites[sPokegearPtr->ArrowSpriteIds[1]].invisible = TRUE;
        return;
    }

    for (u8 i = 0; i < PHONECARD_MAX_VISIBLE; ++i)
    {
        u16 idx = sPhoneCardScroll + i;
        u8 y = PHONECARD_ROW_HEIGHT * i;
        const struct PhoneContact* contact;

        if (idx >= PHONECARD_ENTRY_COUNT)
            break;
        contact = PhoneContactAt(idx);
        if (i == sPhoneCardCursor)
        {
            // cursor arrow + highlighted (red) name
            WindowPrint(WIN_PH_LIST, 1, PHONECARD_CURSOR_X, y, &sPhoneSelText, 0, sText_PhoneCursor);
            WindowPrint(WIN_PH_LIST, 1, PHONECARD_NAME_X, y, &sPhoneSelText, 0, contact->name);
        }
        else
        {
            WindowPrint(WIN_PH_LIST, 1, PHONECARD_NAME_X, y, &sWhiteText, 0, contact->name);
        }
        WindowPrint(WIN_PH_LIST, 1, PHONECARD_ROLE_X, y, &sPhoneRoleText, 0, contact->role);
    }
    CommitWindow(WIN_PH_LIST);

    PrintPhoneBox(FALSE);

    // Scroll arrows
    if (sPokegearPtr->ArrowSpriteIds[0] < MAX_SPRITES)
        gSprites[sPokegearPtr->ArrowSpriteIds[0]].invisible = (sPhoneCardScroll == 0);
    if (sPokegearPtr->ArrowSpriteIds[1] < MAX_SPRITES)
        gSprites[sPokegearPtr->ArrowSpriteIds[1]].invisible = (sPhoneCardScroll + PHONECARD_MAX_VISIBLE >= PHONECARD_ENTRY_COUNT);

    // Portrait (overworld sprite) follows the highlighted contact
    if (selected != sPhoneCardPortraitIdx)
    {
        sPhoneCardPortraitIdx = selected;
        CreatePhoneCardPortrait(PhoneContactAt(selected)->graphicsId);
    }
}

// ---------------------------------------------------------------------------
// Tasks
// ---------------------------------------------------------------------------
static void Task_PhoneCardFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_PhoneCardInputHandler;
}

static void Task_PhoneCardInputHandler(u8 taskId)
{
    if (sPhoneCardMsgActive)
    {
        // message is typing out in the top box; A/B closes it once it is complete
        RunTextPrinters();
        if (!IsTextPrinterActive(WIN_PH_BOX) && (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON)))
        {
            PlaySE(SE_SELECT);
            sPhoneCardMsgActive = FALSE;
            PrintPhoneBox(FALSE);
        }
        return;
    }

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_PhoneCardFadeOutToPokegear;
    }
    else if (PHONECARD_ENTRY_COUNT == 0)
    {
        return; // nothing registered: only B does anything
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (sPhoneCardCursor > 0)
            sPhoneCardCursor--;
        else if (sPhoneCardScroll > 0)
            sPhoneCardScroll--;
        else
            return;
        PlaySE(SE_SELECT);
        PrintPhoneCardUI();
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if ((sPhoneCardScroll + sPhoneCardCursor + 1) >= PHONECARD_ENTRY_COUNT)
            return;
        if (sPhoneCardCursor < PHONECARD_MAX_VISIBLE - 1)
            sPhoneCardCursor++;
        else
            sPhoneCardScroll++;
        PlaySE(SE_SELECT);
        PrintPhoneCardUI();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sPhoneCardMsgActive = TRUE;
        PrintPhoneBox(TRUE);
    }
}

static void Task_PhoneCardFadeOutToPokegear(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyPhoneCardPortrait();
        // Free first, THEN let CB2_ReturnToFieldWithOpenMenu allocate a fresh struct
        // (doing it the other way round frees the struct the Pokegear is about to use).
        FreeAndClosePokegear(taskId);
        CB2_ReturnToFieldWithOpenMenu();
    }
}


// ===========================================================================
//  RADIO CARD
//  Tune with Left/Right (hold to speed up), L/R jump to the next station,
//  A switches the radio on/off, B leaves. Stations come from RadioStations.h.
// ===========================================================================

extern const u8 gText_Radio_FM[];
extern const u8 gText_Radio_MHz[];
extern const u8 gText_Radio_OnAir[];
extern const u8 gText_Radio_StatusStatic[];
extern const u8 gText_Radio_StatusOff[];
extern const u8 gText_Radio_StripStatic[];
extern const u8 gText_Radio_StripOff[];
extern const u8 gText_Radio_BodyStatic[];
extern const u8 gText_Radio_BodyOff[];
extern const u8 gText_Radio_NoProgram[];
extern const u8 gText_Radio_Hint[];
extern const u8 gText_Radio_Scale90[];
extern const u8 gText_Radio_Scale95[];
extern const u8 gText_Radio_Scale100[];
extern const u8 gText_Radio_Scale105[];

static void CB2_RadioCard(void);
static void LoadRadioCardBgGfx(void);
static void InitRadioCardGUI(void);
static void Task_RadioCardFadeIn(u8 taskId);
static void Task_RadioCardMain(u8 taskId);
static void Task_RadioCardFadeOutToPokegear(u8 taskId);

enum RadioWindows
{
    WIN_RA_BOX,      // top box: DJ name + program text (typewriter)
    WIN_RA_READOUT,  // "FM 89.3 MHz" + ON AIR / STATIC / OFF
    WIN_RA_SCALE,    // numbers under the dial
    WIN_RA_HINT,     // text on the bottom bar
    WIN_RA_COUNT
};

static const struct WindowTemplate sRadioWindowTemplates[] =
{
    [WIN_RA_BOX] =
    {
        .bg = BG_TEXT, .tilemapLeft = 5, .tilemapTop = 0, .width = 24, .height = 7,
        .paletteNum = 14, .baseBlock = 1,       // 168 tiles
    },
    [WIN_RA_READOUT] =
    {
        .bg = BG_TEXT, .tilemapLeft = 1, .tilemapTop = 7, .width = 28, .height = 2,
        .paletteNum = 14, .baseBlock = 169,     // 56 tiles
    },
    [WIN_RA_SCALE] =
    {
        .bg = BG_TEXT, .tilemapLeft = 1, .tilemapTop = 15, .width = 28, .height = 2,
        .paletteNum = 14, .baseBlock = 225,     // 56 tiles
    },
    [WIN_RA_HINT] =
    {
        .bg = BG_TEXT, .tilemapLeft = 2, .tilemapTop = 18, .width = 26, .height = 2,
        .paletteNum = 14, .baseBlock = 281,     // 52 tiles
    },
    DUMMY_WIN_TEMPLATE,
};

#define RADIO_NEEDLE_Y      100     // needle / station markers (sprites) on the dial
#define RADIO_MARKER_Y      92
#define RADIO_STATE_NONE    0xFF    // radioState: on, but between stations (static)
#define RADIO_STATE_OFF     0xFE    // radioState: switched off
#define RADIO_STATE_INIT    0xFD    // radioState: forces the first update

#define sRadioFreq          (sPokegearPtr->radioFreq)
#define sRadioSong          (sPokegearPtr->radioSong)
#define sRadioTimer         (sPokegearPtr->radioTimer)
#define sRadioState         (sPokegearPtr->radioState)
#define sRadioOn            (sPokegearPtr->radioOn)
#define sRadioMusicChanged  (sPokegearPtr->radioMusicChanged)
#define sRadioHold          (sPokegearPtr->radioHold)
#define sRadioLine          (sPokegearPtr->radioLine)
#define sRadioLineCount     (sPokegearPtr->radioLineCount)

// colours
static const struct TextColor sRadioGreenText =
{
    .bgColor = TEXT_COLOR_TRANSPARENT,
    .fgColor = TEXT_COLOR_LIGHT_GREEN,
    .shadowColor = TEXT_COLOR_DARK_GRAY,
};

// sprites: tuner needle + station markers
static const struct OamData sRadioNeedleOam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(8x32),
    .size = SPRITE_SIZE(8x32),
    .priority = 1,
};

static const struct OamData sRadioMarkerOam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 1,
};

static const struct SpriteTemplate sRadioNeedleTemplate =
{
    .tileTag = GFXTAG_RADIO_NEEDLE,
    .paletteTag = GFXTAG_RADIO_NEEDLE,
    .oam = &sRadioNeedleOam,
    .anims = sAnimTable_PhoneStill,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sRadioMarkerTemplate =
{
    .tileTag = GFXTAG_RADIO_MARKER,
    .paletteTag = GFXTAG_RADIO_MARKER,
    .oam = &sRadioMarkerOam,
    .anims = sAnimTable_PhoneStill,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

// ---------------------------------------------------------------------------
// Station helpers
// ---------------------------------------------------------------------------
static u16 RadioFreqToX(u16 freq)
{
    return RADIO_DIAL_X0 + ((freq - RADIO_FREQ_MIN) * RADIO_DIAL_W) / (RADIO_FREQ_MAX - RADIO_FREQ_MIN);
}

static const struct RadioStation* RadioStationAt(u8 availPos)
{
    return &sRadioStations[sPokegearPtr->radioAvail[availPos]];
}

// stations that are receivable right now (unlock flag set), sorted by frequency
static void BuildRadioAvailList(void)
{
    u8 n = 0;

    for (u32 i = 0; i < ARRAY_COUNT(sRadioStations); ++i)
    {
        if (PhoneFlagCheck(sRadioStations[i].unlockFlag))
            sPokegearPtr->radioAvail[n++] = i;
    }
    for (u8 a = 1; a < n; ++a)
    {
        u8 v = sPokegearPtr->radioAvail[a];
        s8 b = a - 1;
        while (b >= 0 && sRadioStations[sPokegearPtr->radioAvail[b]].freq > sRadioStations[v].freq)
        {
            sPokegearPtr->radioAvail[b + 1] = sPokegearPtr->radioAvail[b];
            b--;
        }
        sPokegearPtr->radioAvail[b + 1] = v;
    }
    sPokegearPtr->radioAvailCount = n;
}

// which station (list position) is received at this frequency? RADIO_STATE_NONE if none
static u8 FindTunedStation(u16 freq)
{
    for (u8 i = 0; i < sPokegearPtr->radioAvailCount; ++i)
    {
        u16 f = RadioStationAt(i)->freq;
        u16 diff = (freq > f) ? (freq - f) : (f - freq);
        if (diff <= RADIO_LOCK_RANGE)
            return i;
    }
    return RADIO_STATE_NONE;
}

// lines of the tuned station whose flag is set
static void BuildRadioLines(const struct RadioStation* st)
{
    u8 n = 0;

    for (u8 i = 0; i < st->lineCount && n < RADIO_MAX_LINES; ++i)
    {
        if (PhoneFlagCheck(st->lines[i].flag))
            sPokegearPtr->radioLines[n++] = i;
    }
    sRadioLineCount = n;
}

// "893" -> "89.3", "1013" -> "101.3" (digits are charmap codes: 0xA1 = '0', 0xAD = '.')
static void FormatRadioFreq(u8* dst, u16 freq)
{
    u16 whole = freq / 10;
    u8 n = 0;

    if (whole >= 100)
        dst[n++] = 0xA1 + whole / 100;
    if (whole >= 10)
        dst[n++] = 0xA1 + (whole / 10) % 10;
    dst[n++] = 0xA1 + whole % 10;
    dst[n++] = 0xAD;
    dst[n++] = 0xA1 + freq % 10;
    dst[n] = 0xFF;
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------
static void RadioPrintBox(const u8* name, const u8* body, const struct TextColor* bodyColor, bool8 typed)
{
    CleanWindow(WIN_RA_BOX);
    WindowPrint(WIN_RA_BOX, 1, PHONECARD_BOX_X, PHONECARD_BOX_NAME_Y, &sWhiteText, 0, name);
    WindowPrint(WIN_RA_BOX, 1, PHONECARD_BOX_X, PHONECARD_BOX_MSG_Y, bodyColor, typed ? PHONECARD_TEXT_SPEED : 0, body);
    CommitWindow(WIN_RA_BOX);
}

static void RadioPrintReadout(void)
{
    u8 buf[8];
    const u8* status;
    const struct TextColor* statusColor;

    if (sRadioState == RADIO_STATE_OFF)
    {
        status = gText_Radio_StatusOff;
        statusColor = &sPhoneSelText;
    }
    else if (sRadioState == RADIO_STATE_NONE)
    {
        status = gText_Radio_StatusStatic;
        statusColor = &sPhoneRoleText;
    }
    else
    {
        status = gText_Radio_OnAir;
        statusColor = &sRadioGreenText;
    }

    FormatRadioFreq(buf, sRadioFreq);
    CleanWindow(WIN_RA_READOUT);
    WindowPrint(WIN_RA_READOUT, 1, 6, 1, &sWhiteText, 0, gText_Radio_FM);
    WindowPrint(WIN_RA_READOUT, 1, 26, 1, &sWhiteText, 0, buf);
    WindowPrint(WIN_RA_READOUT, 1, 62, 1, &sWhiteText, 0, gText_Radio_MHz);
    WindowPrint(WIN_RA_READOUT, 1, 160, 1, statusColor, 0, status);
    CommitWindow(WIN_RA_READOUT);
}

static void RadioMoveNeedle(void)
{
    if (sPokegearPtr->radioNeedleSprite < MAX_SPRITES)
        gSprites[sPokegearPtr->radioNeedleSprite].pos1.x = RadioFreqToX(sRadioFreq);
}

// show the current line of the tuned station
static void RadioShowLine(void)
{
    const struct RadioStation* st = RadioStationAt(sRadioState);

    if (sRadioLineCount == 0)
        RadioPrintBox(st->name, gText_Radio_NoProgram, &sWhiteText, FALSE);
    else
        RadioPrintBox(st->name, st->lines[sPokegearPtr->radioLines[sRadioLine]].text, &sWhiteText, TRUE);
    sRadioTimer = 0;
}

// ---------------------------------------------------------------------------
// Music: the radio takes over the background music while it is open
// ---------------------------------------------------------------------------
static void RadioApplyMusic(u8 state)
{
    u16 song = 0;

    if (state != RADIO_STATE_OFF && state != RADIO_STATE_NONE)
        song = RadioStationAt(state)->song;

    if (song == sRadioSong && sRadioMusicChanged)
        return; // already in that state

    if (song != 0)
        PlayBGM(song);
    else
        m4aSongNumStop(sRadioSong != 0 ? sRadioSong : GetCurrentMapMusic());

    sRadioSong = song;
    sRadioMusicChanged = TRUE;
}

// ---------------------------------------------------------------------------
// Tuning
// ---------------------------------------------------------------------------
static void RadioUpdateStation(bool8 force)
{
    u8 newState = sRadioOn ? FindTunedStation(sRadioFreq) : RADIO_STATE_OFF;

    if (!force && newState == sRadioState)
        return;

    sRadioState = newState;
    sRadioLine = 0;
    sRadioTimer = 0;
    RadioApplyMusic(newState);

    if (newState == RADIO_STATE_OFF)
    {
        DestroyPhoneCardPortrait();
        RadioPrintBox(gText_Radio_StripOff, gText_Radio_BodyOff, &sPhoneRoleText, FALSE);
    }
    else if (newState == RADIO_STATE_NONE)
    {
        DestroyPhoneCardPortrait();
        RadioPrintBox(gText_Radio_StripStatic, gText_Radio_BodyStatic, &sPhoneRoleText, FALSE);
    }
    else
    {
        const struct RadioStation* st = RadioStationAt(newState);

        BuildRadioLines(st);
        CreatePhoneCardPortrait(st->graphicsId); // DJ sprite from the overworld table
        RadioShowLine();
    }
    RadioPrintReadout();
}

// jump to the next (dir = 1) or previous (dir = -1) station, wrapping around
static void RadioJump(s8 dir)
{
    u8 count = sPokegearPtr->radioAvailCount;
    // start from the station we are tuned to (so L/R never snaps back onto it)
    u16 ref = (sRadioState < count) ? RadioStationAt(sRadioState)->freq : sRadioFreq;
    s16 target = -1;

    if (count == 0)
        return;

    if (dir > 0)
    {
        for (u8 i = 0; i < count; ++i)
        {
            if (RadioStationAt(i)->freq > ref)
            {
                target = i;
                break;
            }
        }
        if (target < 0)
            target = 0;             // wrap to the first station
    }
    else
    {
        for (s8 i = count - 1; i >= 0; --i)
        {
            if (RadioStationAt(i)->freq < ref)
            {
                target = i;
                break;
            }
        }
        if (target < 0)
            target = count - 1;     // wrap to the last station
    }
    sRadioFreq = RadioStationAt(target)->freq;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
static void ShowRadioCard(void)
{
    QuestLog_CutRecording();
    // The Pokegear freed its resources before running this option, so make new ones.
    sPokegearPtr = Calloc(sizeof(struct PokegearResources));
    gMain.state = 0;
    SetMainCallback2(CB2_RadioCard);
}

static void CB2_RadioCard(void)
{
    switch (gMain.state)
    {
        case 0:
        default:
            SetVBlankCallback(NULL);
            ClearVramOamPlttRegs();
            SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_ON);
            gMain.state++;
            break;
        case 1:
            ClearTasksAndGraphicalStructs();
            gMain.state++;
            break;
        case 2:
            sPokegearPtr->sBgTilemapBuffer = Malloc(0x1000);
            ResetBgsAndClearDma3BusyFlags(0);
            InitBgsFromTemplates(0, sPokegearBgTemplates, NELEMS(sPokegearBgTemplates));
            SetBgTilemapBuffer(BG_BACKGROUND, sPokegearPtr->sBgTilemapBuffer);
            gMain.state++;
            break;
        case 3:
            LoadRadioCardBgGfx();
            gMain.state++;
            break;
        case 4:
            if (!free_temp_tile_data_buffers_if_possible())
            {
                ShowBg(BG_TEXT);
                ShowBg(BG_BACKGROUND);
                CopyBgTilemapBufferToVram(BG_BACKGROUND);
                gMain.state++;
            }
            break;
        case 5:
            Free(sPokegearPtr->sBgTilemapBuffer);
            InitWindows(sRadioWindowTemplates);
            DeactivateAllTextPrinters();
            gMain.state++;
            break;
        case 6:
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
            gMain.state++;
            break;
        case 7:
            SetVBlankCallback(VBlankCB_Pokegear);
            InitRadioCardGUI();
            CreateTask(Task_RadioCardFadeIn, 0);
            SetMainCallback2(MainCB2_Pokegear);
            gMain.state = 0;
            break;
    }
}

static void LoadRadioCardBgGfx(void)
{
    DecompressAndCopyTileDataToVram(BG_BACKGROUND, RadioCardBgTiles, 0, 0, 0);
    LZDecompressWram(RadioCardBgMap, sPokegearPtr->sBgTilemapBuffer);
    LoadPalette(sRadioCardSkinPal[PokegearGetDesign()], 0, 0x20);   // colours of the chosen design
    LoadPalette(StdTextPal, 14 * 16, 0x20); // text palette used by every window
}

static void InitRadioCardGUI(void)
{
    u16 startFreq = 0;

    for (u8 i = 0; i < WIN_RA_COUNT; ++i)
    {
        CleanWindow(i);
        PutWindowTilemap(i);
    }

    BuildRadioAvailList();

    // starting frequency: the remembered one, otherwise the first station
#if RADIO_FREQ_VAR
    startFreq = VarGet(RADIO_FREQ_VAR);
#endif
    if (startFreq < RADIO_FREQ_MIN || startFreq > RADIO_FREQ_MAX)
        startFreq = (sPokegearPtr->radioAvailCount > 0) ? RadioStationAt(0)->freq : (RADIO_FREQ_MIN + RADIO_FREQ_MAX) / 2;

    sRadioFreq = startFreq;
    sRadioOn = TRUE;
    sRadioState = RADIO_STATE_INIT;
    sRadioSong = 0;
    sRadioMusicChanged = FALSE;
    sRadioHold = 0;
    sPhoneCardPortraitSpriteId = MAX_SPRITES;

    // tuner needle + one marker per receivable station
    LoadSpriteSheet(&(struct SpriteSheet){radio_needleTiles, 8 * 32 / 2, GFXTAG_RADIO_NEEDLE});
    LoadSpritePalette(&(struct SpritePalette){radio_needlePal, GFXTAG_RADIO_NEEDLE});
    LoadSpriteSheet(&(struct SpriteSheet){radio_markerTiles, 8 * 8 / 2, GFXTAG_RADIO_MARKER});
    LoadSpritePalette(&(struct SpritePalette){radio_markerPal, GFXTAG_RADIO_MARKER});
    for (u8 i = 0; i < sPokegearPtr->radioAvailCount; ++i)
        sPokegearPtr->radioMarkerSprites[i] = CreateSprite(&sRadioMarkerTemplate, RadioFreqToX(RadioStationAt(i)->freq), RADIO_MARKER_Y, 1);
    sPokegearPtr->radioNeedleSprite = CreateSprite(&sRadioNeedleTemplate, RadioFreqToX(sRadioFreq), RADIO_NEEDLE_Y, 0);

    // static texts: numbers under the dial + hint on the bottom bar
    WindowPrint(WIN_RA_SCALE, 0, 26, 0, &sPhoneRoleText, 0, gText_Radio_Scale90);
    WindowPrint(WIN_RA_SCALE, 0, 76, 0, &sPhoneRoleText, 0, gText_Radio_Scale95);
    WindowPrint(WIN_RA_SCALE, 0, 121, 0, &sPhoneRoleText, 0, gText_Radio_Scale100);
    WindowPrint(WIN_RA_SCALE, 0, 171, 0, &sPhoneRoleText, 0, gText_Radio_Scale105);
    CommitWindow(WIN_RA_SCALE);
    WindowPrint(WIN_RA_HINT, 0, 4, 1, &sPhoneDarkText, 0, gText_Radio_Hint);
    CommitWindow(WIN_RA_HINT);

    RadioMoveNeedle();
    RadioUpdateStation(TRUE);
}

// ---------------------------------------------------------------------------
// Tasks
// ---------------------------------------------------------------------------
static void Task_RadioCardFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_RadioCardMain;
}

static void Task_RadioCardMain(u8 taskId)
{
    s8 dir = 0;
    u16 oldFreq = sRadioFreq;

    RunTextPrinters(); // typewriter text

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_RadioCardFadeOutToPokegear;
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        // radio on / off
        PlaySE(SE_SELECT);
        sRadioOn = !sRadioOn;
        RadioUpdateStation(FALSE);
        return;
    }

    if (sRadioOn)
    {
        // L / R: jump to the next or previous station
        if (JOY_NEW(L_BUTTON))
            RadioJump(-1);
        else if (JOY_NEW(R_BUTTON))
            RadioJump(1);

        // Left / Right: fine tuning, speeds up while held
        if (JOY_HELD(DPAD_LEFT | DPAD_RIGHT))
        {
            if (sRadioHold < 255)
                sRadioHold++;
        }
        else
        {
            sRadioHold = 0;
        }
        if (JOY_NEW_AND_REPEATED(DPAD_RIGHT))
            dir = 1;
        else if (JOY_NEW_AND_REPEATED(DPAD_LEFT))
            dir = -1;
        if (dir != 0)
        {
            u8 step = (sRadioHold >= 60) ? 5 : (sRadioHold >= 30) ? 2 : 1;
            s16 f = sRadioFreq + dir * step;

            if (f < RADIO_FREQ_MIN)
                f = RADIO_FREQ_MIN;
            if (f > RADIO_FREQ_MAX)
                f = RADIO_FREQ_MAX;
            sRadioFreq = f;
        }

        if (sRadioFreq != oldFreq)
        {
            RadioMoveNeedle();
            RadioPrintReadout();
            RadioUpdateStation(FALSE);
        }
    }

    // program: after a line is fully shown and has stayed a while, play the next one
    if (sRadioState < sPokegearPtr->radioAvailCount && sRadioLineCount > 1)
    {
        if (IsTextPrinterActive(WIN_RA_BOX))
        {
            sRadioTimer = 0;
        }
        else if (++sRadioTimer >= RADIO_LINE_FRAMES)
        {
            sRadioLine = (sRadioLine + 1) % sRadioLineCount;
            RadioShowLine();
        }
    }
}

static void Task_RadioCardFadeOutToPokegear(u8 taskId)
{
    if (!gPaletteFade.active)
    {
#if RADIO_FREQ_VAR
        VarSet(RADIO_FREQ_VAR, sRadioFreq);
#endif
        DestroyPhoneCardPortrait();
        // give the map music back if the radio changed it
        if (sRadioMusicChanged)
            PlayNewMapMusic(GetCurrentMapMusic());
        // Free first, THEN let CB2_ReturnToFieldWithOpenMenu allocate a fresh struct
        FreeAndClosePokegear(taskId);
        CB2_ReturnToFieldWithOpenMenu();
    }
}
