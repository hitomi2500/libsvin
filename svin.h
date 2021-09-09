#ifndef _SVIN_H_
#define _SVIN_H_

//#define PORT_USE_FILECLIENT

#include <yaul.h>
#include <svin_textbox.h>
#include <svin_script.h>
#include <svin_filelist.h>
#include <svin_sprite.h>

//#define _SVIN_DIRTY_STATIC_LINKING

#define _SVIN_SCREEN_WIDTH    704
#define _SVIN_SCREEN_HEIGHT   448

#define _SVIN_CHARACTER_HEIGHT   8
#define _SVIN_CHARACTER_WIDTH   8
#define _SVIN_CHARACTER_BYTES   (_SVIN_CHARACTER_HEIGHT*_SVIN_CHARACTER_WIDTH)
#define _SVIN_CHARACTER_UNITS   (_SVIN_CHARACTER_BYTES/32)

//using VDP2 VRAM static allocation with everything contingent
// Bank 0
//  0x00000000 - 0x0001FFFF NBG0 character pattern name data (up to 2048 8x8 tiles) = 0x20000
// Bank 1
//  0x00020000 - 0x00037FFF NBG0 character pattern name data (up to 2048 8x8 tiles) = 0x20000
// Bank 2
//  0x00040000 - 0x00047FFF NBG0 pattern name data 128x64*4 = 0x8000
//  0x00048000 - 0x0004FFFF NBG1 pattern name data 128x64*4 = 0x8000
//  0x00050000 - 0x0005FFFF NBG1 character pattern name data (up to 1024 8x8 tiles) = 0x10000
// Bank 3
//  0x00060000 - 0x0006FFFF NBG1 character pattern name data (up to 1024 8x8 tiles) = 0x10000
//  0x00070000 - 0x0007EFFF NBG1 rendered font data for dialog box 640x80 (40x5*16x16) = 0x????
//  0x0007F000 - 0x0007FFFF NBG1 character pattern name data specials

#define _SVIN_NBG0_CHPNDR_START (VDP2_VRAM_ADDR(0,0))
#define _SVIN_NBG0_CHPNDR_SIZE (0x40000)
#define _SVIN_NBG0_PNDR_START (VDP2_VRAM_ADDR(2,0))
#define _SVIN_NBG0_PNDR_SIZE (128*64*4)
#define _SVIN_NBG1_CHPNDR_START (VDP2_VRAM_ADDR(2,0x8000))
#define _SVIN_NBG1_CHPNDR_SIZE (0x20000)
#define _SVIN_NBG1_PNDR_START (VDP2_VRAM_ADDR(2,0x10000))
#define _SVIN_NBG1_PNDR_SIZE (128*64*4)
#define _SVIN_NBG1_CHPNDR_TEXTBOX_ADDR (VDP2_VRAM_ADDR(3,0x10000))
#define _SVIN_NBG1_CHPNDR_TEXTBOX_INDEX ((0x70000)/32)
#define _SVIN_NBG1_CHPNDR_SPECIALS_ADDR (VDP2_VRAM_ADDR(3,0x1F000))
#define _SVIN_NBG1_CHPNDR_SPECIALS_INDEX ((0x7F000)/32)

//VDP1 command list order
#define _SVIN_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX  0
#define _SVIN_VDP1_ORDER_LOCAL_COORDS_A_INDEX      1
#define _SVIN_VDP1_ORDER_SPRITE_A0_INDEX           2
#define _SVIN_VDP1_ORDER_SPRITE_A1_INDEX           3
#define _SVIN_VDP1_ORDER_SPRITE_A2_INDEX           4
#define _SVIN_VDP1_ORDER_SPRITE_A3_INDEX           5
#define _SVIN_VDP1_ORDER_DRAW_END_A_INDEX          6
#define _SVIN_VDP1_ORDER_LOCAL_COORDS_B_INDEX      7
#define _SVIN_VDP1_ORDER_SPRITE_B0_INDEX           8
#define _SVIN_VDP1_ORDER_SPRITE_B1_INDEX           9
#define _SVIN_VDP1_ORDER_SPRITE_B2_INDEX           10
#define _SVIN_VDP1_ORDER_SPRITE_B3_INDEX           11
#define _SVIN_VDP1_ORDER_DRAW_END_B_INDEX          12
#define _SVIN_VDP1_ORDER_COUNT                     13

void _svin_init();
void _svin_delay(int milliseconds);
void _svin_wait_for_key_press_and_release();

void _svin_background_fade_to_black_step();
void _svin_background_fade_to_black();
void _svin_background_set_palette(int number, uint8_t * pointer);
void _svin_background_clear_palette(int number);
void _svin_background_set_by_index(int index);
void _svin_background_set(char *name);
void _svin_background_update_by_index(int index);
void _svin_background_update(char *name);
void _svin_background_clear();
void _svin_background_fade_to_black();
void _svin_background_load_index(char * filename);
//void _svin_background_set_by_index_half(int index, int part, int slot);

void _svin_tapestry_init();
void _svin_tapestry_load_position(iso9660_filelist_t *_filelist, char *filename, int position);
void _svin_tapestry_move_up();
void _svin_tapestry_move_down();



//void _svin_actor_load_index(iso9660_filelist_t * _filelist, char * actor_code);
void _svin_actor_debug_load_index(iso9660_filelist_t *_filelist);
void _svin_actor_debug_load_test(iso9660_filelist_t *_filelist, char *filename, int actor_id);


#endif
