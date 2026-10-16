#ifndef GUARD_PHONE_CONTACTS_H
#define GUARD_PHONE_CONTACTS_H

// =============================================================================
//  PHONE CARD CONTACT TABLE  -  this is the only file you need to edit to add
//  or change contacts.
//
//  1. REGISTERED FLAG
//     A contact only shows up in the phone card once its registered flag is set
//     (like registering a trainer in the original games). Set it from a script:
//         setflag FLAG_PHONE_BROCK
//     PHONE_ALWAYS = visible from the very start.
//
//  2. MESSAGES BY GAME STATE
//     Every contact has a list of messages. The list is checked from TOP to
//     BOTTOM and the FIRST entry whose flag is set is the one that is shown.
//     So: put the most advanced state first, and finish with a PHONE_ALWAYS
//     entry as the fallback.
//        { FLAG_BADGE08_GET, gText_PhoneMsg_Brock_Badges },   // checked first
//        { FLAG_BADGE01_GET, gText_PhoneMsg_Brock_Beaten  },
//        { PHONE_ALWAYS,     gText_NPCMsg5                },   // fallback
//     PHONE_NEVER = never matches (handy as a placeholder for a flag you have
//     not picked yet).
//
//  3. TEXT
//     All text lives in strings/PokeGear.string (#org @label). Declare each
//     label in the "text labels" list below, then use it in the table.
//     Messages may use \n for a line break (2 lines of ~24 characters fit).
//
//  4. PORTRAIT
//     graphicsId is any index of your overworld sprite table.
// =============================================================================

#include "../include/constants/flags.h"
#include "../include/constants/event_objects.h"

#define PHONE_ALWAYS 0x0000   // condition is always true
#define PHONE_NEVER  0xFFFF   // condition is never true

struct PhoneMessage
{
    u16 flag;           // shown when this flag is set (or PHONE_ALWAYS)
    const u8* text;
};

struct PhoneContact
{
    const u8* name;
    const u8* role;     // second column of the list ("Leader", "Professor", ...)
    u16 graphicsId;     // overworld sprite table index (portrait)
    u16 registeredFlag; // contact is listed once this flag is set (or PHONE_ALWAYS)
    const struct PhoneMessage* messages;
    u8 messageCount;
};

// ---- registered flags --------------------------------------------------------
// PLACEHOLDERS: they are PHONE_ALWAYS so every contact is visible until you
// replace them with your own (unused!) flag numbers, e.g.  0x????
#define FLAG_PHONE_MOM       PHONE_ALWAYS
#define FLAG_PHONE_OAK       PHONE_ALWAYS
#define FLAG_PHONE_DAISY     PHONE_ALWAYS
#define FLAG_PHONE_BILL      PHONE_ALWAYS
#define FLAG_PHONE_BROCK     PHONE_ALWAYS
#define FLAG_PHONE_MISTY     PHONE_ALWAYS
#define FLAG_PHONE_SURGE     PHONE_ALWAYS
#define FLAG_PHONE_ERIKA     PHONE_ALWAYS

// ---- text labels (defined in strings/PokeGear.string) -------------------------
extern const u8 gText_NPCName1[];
extern const u8 gText_NPCName2[];
extern const u8 gText_NPCName3[];
extern const u8 gText_NPCName4[];
extern const u8 gText_NPCName5[];
extern const u8 gText_NPCName6[];
extern const u8 gText_NPCName7[];
extern const u8 gText_NPCName8[];
extern const u8 gText_NPCRole1[];
extern const u8 gText_NPCRole2[];
extern const u8 gText_NPCRole3[];
extern const u8 gText_NPCRole4[];
extern const u8 gText_NPCRole5[];
extern const u8 gText_NPCRole6[];
extern const u8 gText_NPCRole7[];
extern const u8 gText_NPCRole8[];
extern const u8 gText_NPCMsg1[];
extern const u8 gText_NPCMsg2[];
extern const u8 gText_NPCMsg3[];
extern const u8 gText_NPCMsg4[];
extern const u8 gText_NPCMsg5[];
extern const u8 gText_NPCMsg6[];
extern const u8 gText_NPCMsg7[];
extern const u8 gText_NPCMsg8[];
extern const u8 gText_PhoneMsg_Oak_Champion[];
extern const u8 gText_PhoneMsg_Brock_Beaten[];
extern const u8 gText_PhoneMsg_Brock_Badges[];

// ---- messages per contact (first matching flag wins, top to bottom) -------------
static const struct PhoneMessage sPhoneMsgs_Mom[] =
{
    { PHONE_ALWAYS,        gText_NPCMsg1 },
};

static const struct PhoneMessage sPhoneMsgs_Oak[] =
{
    { FLAG_SYS_GAME_CLEAR, gText_PhoneMsg_Oak_Champion },   // example state
    { PHONE_ALWAYS,        gText_NPCMsg2 },
};

static const struct PhoneMessage sPhoneMsgs_Daisy[] =
{
    { PHONE_ALWAYS,        gText_NPCMsg3 },
};

static const struct PhoneMessage sPhoneMsgs_Bill[] =
{
    { PHONE_ALWAYS,        gText_NPCMsg4 },
};

static const struct PhoneMessage sPhoneMsgs_Brock[] =
{
    { FLAG_BADGE08_GET,    gText_PhoneMsg_Brock_Badges },   // example states
    { FLAG_BADGE01_GET,    gText_PhoneMsg_Brock_Beaten },
    { PHONE_ALWAYS,        gText_NPCMsg5 },
};

static const struct PhoneMessage sPhoneMsgs_Misty[] =
{
    { PHONE_ALWAYS,        gText_NPCMsg6 },
};

static const struct PhoneMessage sPhoneMsgs_Surge[] =
{
    { PHONE_ALWAYS,        gText_NPCMsg7 },
};

static const struct PhoneMessage sPhoneMsgs_Erika[] =
{
    { PHONE_ALWAYS,        gText_NPCMsg8 },
};

#define PHONE_MSGS(table) table, ARRAY_COUNT(table)

// ---- the contact table (order = order in the list) ---------------------------------
//   name,           role,           portrait,            registered flag,   messages
static const struct PhoneContact sPhoneContacts[] =
{
    { gText_NPCName1, gText_NPCRole1, OBJ_EVENT_GFX_MOM,      FLAG_PHONE_MOM,   PHONE_MSGS(sPhoneMsgs_Mom)   },
    { gText_NPCName2, gText_NPCRole2, OBJ_EVENT_GFX_PROF_OAK, FLAG_PHONE_OAK,   PHONE_MSGS(sPhoneMsgs_Oak)   },
    { gText_NPCName3, gText_NPCRole3, OBJ_EVENT_GFX_DAISY,    FLAG_PHONE_DAISY, PHONE_MSGS(sPhoneMsgs_Daisy) },
    { gText_NPCName4, gText_NPCRole4, OBJ_EVENT_GFX_BILL,     FLAG_PHONE_BILL,  PHONE_MSGS(sPhoneMsgs_Bill)  },
    { gText_NPCName5, gText_NPCRole5, OBJ_EVENT_GFX_BROCK,    FLAG_PHONE_BROCK, PHONE_MSGS(sPhoneMsgs_Brock) },
    { gText_NPCName6, gText_NPCRole6, OBJ_EVENT_GFX_MISTY,    FLAG_PHONE_MISTY, PHONE_MSGS(sPhoneMsgs_Misty) },
    { gText_NPCName7, gText_NPCRole7, OBJ_EVENT_GFX_LT_SURGE, FLAG_PHONE_SURGE, PHONE_MSGS(sPhoneMsgs_Surge) },
    { gText_NPCName8, gText_NPCRole8, OBJ_EVENT_GFX_ERIKA,    FLAG_PHONE_ERIKA, PHONE_MSGS(sPhoneMsgs_Erika) },
};

#endif // GUARD_PHONE_CONTACTS_H
