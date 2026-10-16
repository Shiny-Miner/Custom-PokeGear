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

#define MAX_Pokegear_ITEMS 2
#define cpos sPokegearPtr->cursorpos[0]
#define scrolloffset sPokegearPtr->cursorpos[1]
#define numitems sPokegearPtr->NumPokegearItems[0]
#define numonscreenitems sPokegearPtr->NumPokegearItems[1]
#define menuitems sPokegearPtr->CurrentOptionsTable[0] 
#define onscreenmenuitems sPokegearPtr->CurrentOptionsTable[1]
extern void QuestLog_CutRecording(void);
extern void InitRegionMapWithExitCB(u8 type, void (*callback)(void));


enum WindowIds
{
    WIN_TOPBAR_TIME,
    WIN_TOPBAR,
    WIN_ITEMS,
    WIN_BOTTOMBAR,
    WIN_COUNT
};   


#define PHONECARD_MAX_CONTACTS 64 // size of sPhoneContacts[] must not exceed this
struct PokegearResources
{
  u8 cursorpos[2];
  u8* sBgTilemapBuffer;
  u8 NumPokegearItems[2];
  u8 CurrentOptionsTable[2][MAX_Pokegear_ITEMS]; 
  u8 IconSpriteIds[6];
  u8 ArrowSpriteIds[2];

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
};   

#define sPokegearPtr (*((struct PokegearResources**) 0x203E038))  

extern u16 StdTextPal[];
static const struct WindowTemplate sMenuWindowTemplates[] = 
{
    [WIN_TOPBAR_TIME] = 
    {
        .bg = BG_TEXT,      // which bg to print text on
        .tilemapLeft = 18,   // position from left (per 8 pixels)
        .tilemapTop = 0,    // position from top (per 8 pixels)
        .width = 10,        // width (per 8 pixels)
        .height = 4,        // height (per 8 pixels)
        .paletteNum = 14,   // palette index to use for text
        .baseBlock = 1,     // tile start in VRAM
    }, 
    [WIN_TOPBAR] = 
    {
        .bg = BG_TEXT,      // which bg to print text on
        .tilemapLeft = 10,   // position from left (per 8 pixels)
        .tilemapTop = 0,    // position from top (per 8 pixels)
        .width = 20,        // width (per 8 pixels)
        .height = 4,        // height (per 8 pixels)
        .paletteNum = 14,   // palette index to use for text
        .baseBlock = 42,     // tile start in VRAM
    },                
    [WIN_ITEMS] = 
    {
        .bg = BG_TEXT,      // which bg to print text on
        .tilemapLeft = 0,   // position from left (per 8 pixels)
        .tilemapTop = 4,    // position from top (per 8 pixels)
        .width = 30,        // width (per 8 pixels)
        .height = 12,        // height (per 8 pixels)
        .paletteNum = 14,   // palette index to use for text
        .baseBlock = 123,     // tile start in VRAM
    },            
    [WIN_BOTTOMBAR] = 
    {
        .bg = BG_TEXT,      // which bg to print text on
        .tilemapLeft = 0,   // position from left (per 8 pixels)
        .tilemapTop = 16,    // position from top (per 8 pixels)
        .width = 30,        // width (per 8 pixels)
        .height = 4,        // height (per 8 pixels)
        .paletteNum = 14,   // palette index to use for text
        .baseBlock = 484,     // tile start in VRAM
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
static void DrawPanels(void); 
static void Task_PokegearFadeOut(u8 taskId); 
static void FreeAndClosePokegear(u8 taskId); 
static void PrintGUIMapName(void);
static void CleanWindow(u8 windowId); 
static void CommitWindow(u8 windowId); 
static void CleanWindows(void); 
static void CommitWindows(void); 
static void PrintGUIMenuItemsName(void);
static void DrawIcons(void);
void Pokegear_Init(void); 
static void CreateScrollbar(void) ;
static void CalculateAndConfigureOnScreenOptions(void);
static void Task_RunPokegearOptionFuncOrScript(u8 taskId);
static void RefreshPokegearOptions(void);
static void PrintAndUpdateTimeText();
static void ShowTownMap(void);
static void ShowPhoneCard(void);
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

static const struct PokegearOption sPokegearOptionsTable[] = 
{
  {
    .id =  Pokegear_MAPCARD,
    .text = (u8*) gText_Pokegear_MapCard,
    .flag = 0,
    .script = NULL,
    .func = ShowTownMap
  },
  {
    .id = Pokegear_PHONECARD,
    .text = (u8*) gText_Pokegear_PhoneCard,
    .flag = 0,
    .script = NULL,
    .func = ShowPhoneCard
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
			InitWindows(sMenuWindowTemplates);
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
	{
			gTasks[taskId].func = Task_PokegearWaitForKeyPress;
	}
}  

static void LoadPokegearBgGfx(void)
{
	const u8 *tiles, *map;
	const u16 *palette;
  tiles = PokegearBgTiles; 
  map = PokegearBgMap; 
  palette = PokegearBgPal;
	DecompressAndCopyTileDataToVram(BG_BACKGROUND, tiles, 0, 0, 0);
	LZDecompressWram(map, sPokegearPtr->sBgTilemapBuffer);
	LoadPalette(palette, 0, 0x20); 
	palette = StdTextPal;
	LoadPalette(palette, 14*16, 0x20);
} 

static bool8 InitPokegearGUI(void)
{ 
  CleanWindows();
  DrawPanels(); 
  DrawIcons();
  PrintGUIMapName(); 
  PrintGUIMenuItemsName();
  PrintAndUpdateTimeText();
  CreateScrollbar();
  CommitWindows();
  return TRUE;
}
static void Task_PokegearWaitForKeyPress(u8 taskId)
{ 
  // Handles Input
  if (gClock.second==0)
  {
    CleanWindow(WIN_TOPBAR_TIME);
    PrintAndUpdateTimeText();
    CommitWindow(WIN_TOPBAR_TIME);
  }
  if (JOY_NEW(B_BUTTON)) 
  { 
    cpos = 0xFF;
    PlaySE(SE_PC_OFF);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
		gTasks[taskId].func = Task_PokegearFadeOut;
  } 
  else if(JOY_NEW(A_BUTTON))
  {
    PlaySE(SE_SELECT);
    VarSet(0x8000, cpos);
    VarSet(0x8001, scrolloffset);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_RunPokegearOptionFuncOrScript;
  }
  else if (JOY_NEW(DPAD_RIGHT)) 
  {
    if (!(cpos+1>= numonscreenitems) && !(cpos%2==1))
    {
      PlaySE(SE_SELECT);
      cpos++; 
    }
  } 
  else if (JOY_NEW(DPAD_LEFT))
  {
    if ((cpos-1>=0) && (cpos%2==1))
    {
      PlaySE(SE_SELECT);
      cpos--;
    }
  } 
  else if (JOY_NEW(DPAD_UP)) 
  {
    if (cpos>1 && cpos<4 && scrolloffset>0)
    {
      PlaySE(SE_SELECT);
      scrolloffset -= 1; 
      RefreshPokegearOptions();
    }
    else if (cpos-2>=0)  
    {
      PlaySE(SE_SELECT);
      cpos -= 2; 
    }

  } 
  else if (JOY_NEW(DPAD_DOWN)) 
  {
    if (cpos>1 && cpos<4 && scrolloffset<numitems/2-3+numitems%2)
    {
      scrolloffset += 1; 
      PlaySE(SE_SELECT);
      RefreshPokegearOptions();
    }
    else if (cpos+2<numonscreenitems)  
   {
      PlaySE(SE_SELECT);
      cpos += 2; 
   }
  }
} 


static void Task_RunPokegearOptionFuncOrScript(u8 taskId)
{
  if (!gPaletteFade.active)
  { 
    if (sPokegearOptionsTable[onscreenmenuitems[cpos]].func!=NULL) 
      SetMainCallback2(sPokegearOptionsTable[onscreenmenuitems[cpos]].func);
    else
    {
      ScriptContext1_SetupScript(sPokegearOptionsTable[onscreenmenuitems[cpos]].script); 
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

static void SetUpPokegear_NormalField(void) 
{
  u8 cursor = 0;
  for (u8 i = 0; i < MAX_Pokegear_ITEMS; i++)
  { 
    if (sPokegearOptionsTable[i].flag!=0 && !FlagGet(sPokegearOptionsTable[i].flag)) 
      continue;
    menuitems[cursor] = sPokegearOptionsTable[i].id;
    cursor++;
  } 
  numitems = cursor; 
  if (!scrolloffset)
    scrolloffset = 0;
  CalculateAndConfigureOnScreenOptions();
}


void PanelCallBack(struct Sprite *sprite) 
{
  if (sprite->data[0]>=numonscreenitems && sprite->data[0]!=0xFF)
    sprite->invisible = 1;
  else 
    sprite->invisible = 0;
  if(sprite->data[0] == cpos) 
    StartSpriteAnimIfDifferent(sprite, 1);
  else 
    StartSpriteAnimIfDifferent(sprite, 0);
}

void PokegearIconCallback(struct Sprite *sprite) 
{ 
  if(sprite->data[0] == cpos)
  {
    StartSpriteAnim(sprite, 1);
    if (sprite->data[4]%5==0)
    {
      if(sprite->data[1]==0)
        sprite->data[3] = 1;
      else if(sprite->data[1]==3)
        sprite->data[3] = -1;
      sprite->data[1] += sprite->data[3];
      sprite->pos1.y = sprite->data[2] + sprite->data[1]; 
    }
  sprite->data[4]++;
  }
  else 
  {
    sprite->pos1.y = sprite->data[2];
    StartSpriteAnim(sprite, 0);
  }
}

static void DrawPanels(void)
{ 
  u16 x, y;
  u8 counter = 0; 
  LoadSpriteSheet(&PanelSpriteSheet);
  LoadSpritePalette(&PanelSpritePalette);
  for (u8 j = 0; j<3; j++) 
  {
    for (u8 i = 0; i<2; i++) 
    {
      x = PANEL_X + (HSPACING + 64/2 + 62)*i;
      y = (PANEL_Y + (VSPACING + 32)*j); 
      u8 SpriteId1 = CreateSprite(&sPanel1SpriteTemplate,x, y, 0);
      u8 SpriteId2 = CreateSprite(&sPanel2SpriteTemplate,x+64, y, 0); 
      gSprites[SpriteId1].data[0] = counter;
      gSprites[SpriteId2].data[0] = counter; 
      counter++; 
    }
  }
} 


static void DrawIcons(void) 
{
  u16 x, y; 
  x = 0;
  y = 0;
  u8 counter = 0;
  for (u8 j = 0; j<3; j++) 
  {
    if(counter==numonscreenitems)
        break; 
    for (u8 i = 0; i<2; i++) 
    {
      if(counter==numonscreenitems)
        break; 
      x = (PANEL_X -11) + (HSPACING-2+ 64/2 + 64)*i;
      y = (PANEL_Y-3)+ (VSPACING +32)*j; 
      LoadSpriteSheet(&PokegearIconTable[onscreenmenuitems[counter]].spritesheet); 
      LoadSpritePalette(&PokegearIconTable[onscreenmenuitems[counter]].spritepalette);
      u8 SpriteId = CreateSprite(&PokegearIconTable[onscreenmenuitems[counter]].sprtemplate , x, y, 0);
      gSprites[SpriteId].data[0] = counter;
      gSprites[SpriteId].data[1] = 3;
      gSprites[SpriteId].data[2] = y;
      gSprites[SpriteId].data[3] = -1; 
      sPokegearPtr->IconSpriteIds[counter] = SpriteId;
      counter++; 
    }
  }
}


static void FreeAndClosePokegear(u8 taskId)
{
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

static void PrintGUIMapName(void)
{ 
  	GetMapName(gStringVar4, GetCurrentRegionMapSectionId(), 0); 
  	WindowPrint(WIN_BOTTOMBAR, 0, 2, 13, &sWhiteText, 0, gStringVar4); 
}

static void PrintGUIMenuItemsName(void) 
{ 
  u16 x, y;
  x = 0; 
  y = 0; 
  u8 counter = 0;
  for (u8 row = 0;row<3; row++)
  { 
    if (counter==numonscreenitems) 
      break;
    for (u8 column = 0; column<2; column++) 
    {
      if (counter==numonscreenitems) 
        break; 
      x = (PANEL_X + (HSPACING + 64/2 + 64)*column) + 8;
      y = (PANEL_Y + (VSPACING + 32)*row) - 40;
        WindowPrint(WIN_ITEMS, 1, x, y, &sWhiteText, 0, sPokegearOptionsTable[onscreenmenuitems[counter]].text); 
      counter++;
    }
  }
} 

void CB2_ReturnToFieldWithOpenMenu(void)
{ 
  sPokegearPtr = Calloc(sizeof(struct PokegearResources));
  cpos = VarGet(0x8000); 
  scrolloffset = VarGet(0x8001);
  SetMainCallback2(CB2_Pokegear);
} 


static void CalculateAndConfigureOnScreenOptions(void) 
{
  u8 counter = 0;
  for (u8 i = 0; i<6; i++)
  {
    if (i+2*scrolloffset == numitems)
      break;
    onscreenmenuitems[i] = menuitems[i+scrolloffset*2];
    counter++;
  } 
  numonscreenitems = counter;
}


static void RefreshPokegearOptions(void) 
{
  CleanWindow(WIN_ITEMS);
  for (u8 i=0; i<numonscreenitems; i++) 
    DestroySpriteAndFreeResources(&gSprites[sPokegearPtr->IconSpriteIds[i]]);
  CalculateAndConfigureOnScreenOptions();
  DrawIcons();
  PrintGUIMenuItemsName();
  CommitWindow(WIN_ITEMS);
} 
static void CreateScrollbar(void) 
{
  LoadSpriteSheet(&ScrollBarSpriteSheet);
  LoadSpritePalette(&ScrollBarSpritePalette); 
  CreateSprite(&ScrollBarSpriteTemplate, 240-5, 48, 0);

}

extern const u8 gText_PhoneCard_HintList[];
extern const u8 gText_PhoneCard_Idle[];
extern const u8 gText_PhoneCard_NoReply[];
extern const u8 gText_PhoneCard_NoContacts[];
extern const u8 gText_PhoneCard_NoContactsBox[];

// The contact table (names, portraits, registered flags, messages by flag) lives in its own file
#include "PhoneContacts.h"

_Static_assert(ARRAY_COUNT(sPhoneContacts) <= PHONECARD_MAX_CONTACTS, "too many phone contacts");

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

static void PrintAndUpdateTimeText()
{
	const u8* amPMString = (gClock.hour >= 12) ? gText_PM : gText_AM;
	ConvertIntToDecimalStringN(gStringVar1, (gClock.hour == 0) ? 12 : (gClock.hour > 12) ? gClock.hour - 12 : gClock.hour, STR_CONV_MODE_RIGHT_ALIGN, 2); //Hour - 12hr format
	ConvertIntToDecimalStringN(gStringVar2, gClock.minute, STR_CONV_MODE_LEADING_ZEROS, 2); //Minute
	StringCopy(gStringVar3, sDayNames[gClock.dayOfWeek]); //Day of Week
	StringExpandPlaceholders(gStringVar4, gText_Pokegear_TimeBase_12Hr);
//	AddTextPrinterParameterized(sTimeWindowId, 2, gStringVar4, 4, 3, 0xFF, NULL);
  WindowPrint(WIN_TOPBAR_TIME, 0, 3, 0, &sWhiteText, 0 ,gStringVar4);
  WindowPrint(WIN_TOPBAR_TIME, 0, 63, 0, &sWhiteText, 0 ,amPMString);
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
    LoadPalette(PhoneCardBgPal, 0, 0x20);
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
