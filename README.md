# Custom HGSS PokeGear by Shiny Miner

## Support and doubts : https://discord.gg/zqphPZSzhn

A HGSS-style Pokégear for a FireRed (BPRE) ROM, It has four plates (Map, Radio, Phone, Exit), six colour designs and its own phone and radio cards.

Credits to Shiny Miner(creator), ansh(BW Start menu as base), Dr. Seuss for Pokegear BG

<img width="1366" height="768" alt="image" src="https://github.com/user-attachments/assets/9a022c01-2b43-4899-b8ab-8e2202b97dc1" />
<img width="1366" height="768" alt="image" src="https://github.com/user-attachments/assets/9ec26165-e846-4918-b638-9ef5de48d34e" />
<img width="1366" height="768" alt="image" src="https://github.com/user-attachments/assets/2a64e220-6d1f-48af-b76b-6a08f117417c" />
<img width="1366" height="768" alt="image" src="https://github.com/user-attachments/assets/a25e52f9-0092-4400-8bb8-14b52f4b7e39" />
<img width="1366" height="768" alt="image" src="https://github.com/user-attachments/assets/3d9345bb-eff9-4b78-9ed9-d23c9bd4b7b6" />
<img width="1366" height="768" alt="image" src="https://github.com/user-attachments/assets/0c974830-9587-4f22-9719-9c8a6531d5c7" />
<img width="1366" height="768" alt="image" src="https://github.com/user-attachments/assets/39bec034-08b2-42a2-850e-6dbeac1137c5" />



## Build

build is same as CFRU. If you don't know how, then checkout how, idc.

## In case of using BW start menu
You need to remove both lines from functionrewrites file (make it empty)

## 1. Player controls

• Left / Right: move between plates. Locked plates are skipped.
• L / R: change colour design (Red, Blue, Green, Yellow, Purple, Pink). The cards use the same design.
• A: open the selected card. On Exit, close the Pokégear.
• B: go back or close.
• Phone card: Up/Down to pick a contact, A to call, B to go back.
• Radio card: Left/Right tunes (hold to go faster), L/R jump to the next station, A switches the radio on or off.

## 2. Files

• Pokegear.c: all logic (main screen, phone card, radio card).
• Pokegear.h, graphics.h, PokegearUI.h: declarations, layout and sprite data.
• PokegearSkins.h: colour designs and main settings.
• PhoneContacts.h: contact table.
• RadioStations.h: station table.
• strings/PokeGear.string: all text.
• graphics/: PokegearBg.png and the sprites (pg_map, pg_radio, pg_phone, pg_exit, pg_glow, pg_pip).

## 3. Main settings (PokegearSkins.h)

• POKEGEAR_FLAG_MAP, POKEGEAR_FLAG_RADIO, POKEGEAR_FLAG_PHONE: flag that unlocks each card. 0 means always usable. Example: #define POKEGEAR_FLAG_PHONE FLAG_BADGE02_GET.
• POKEGEAR_DESIGN_VAR: a var that remembers the chosen design. 0 means it is not remembered.
• POKEGEAR_DESIGN_FADE_FRAMES (24): length of the colour fade. 60 frames is one second.
• POKEGEAR_NUM_DESIGNS (6): colours are PG_RGB(r,g,b), each value 0–31, in three tables (main screen, phone/radio card, dots).

## 4. Phone contacts (PhoneContacts.h)

Each contact has a name, role, portrait (OBJ_EVENT_GFX_*), a registered flag and a message list.
• A contact is listed once its registered flag is set. The FLAG_PHONE_* entries are placeholders set to PHONE_ALWAYS. Replace them with your own unused flags and use setflag in a script to register someone.
• Messages are checked top to bottom and the first set flag wins. Put the most advanced state first and end with PHONE_ALWAYS.
• Maximum 255 (above untested) contacts. Messages fit 2 lines of about 24 characters.

## 5. Radio stations (RadioStations.h)

• Dial: 88.0–108.0 MHz (RADIO_FREQ_MIN 880, RADIO_FREQ_MAX 1080). A station is received within ±0.3 MHz (RADIO_LOCK_RANGE 3). Keep stations about 1 MHz apart.
• RADIO_LINE_FRAMES (150): how long a finished line stays on screen.
• RADIO_FREQ_VAR: var that remembers the tuned frequency (0 = off).
• Each station has a name, frequency (893 = 89.3), unlock flag, MUS_* song, DJ portrait and a list of lines. Lines with a set flag play in order and then loop.
• Maximum 16 stations and 16 lines per station.
Default stations: Pokemon Talk 89.3 (always), Pokemon Music 92.5 (Pokédex), Poke Flute 96.8 (Poké Flute), Lucky Channel 101.3 (game clear).

## 6. Text (strings/PokeGear.string)

All text uses #org @label. Declare every new label with extern const u8 label[]; in the header that uses it. Covers card names, day and AM/PM, contact names, roles and messages, radio station names and lines, and hints.
