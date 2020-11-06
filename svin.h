#ifndef _SVIN_H_
#define _SVIN_H_

#include <yaul.h>

#define _SVIN_SCREEN_WIDTH    704
#define _SVIN_SCREEN_HEIGHT   448

#define _SVIN_CHARACTER_HEIGHT   16
#define _SVIN_CHARACTER_WIDTH   16
#define _SVIN_CHARACTER_BYTES   (_SVIN_CHARACTER_HEIGHT*_SVIN_CHARACTER_WIDTH)
#define _SVIN_CHARACTER_UNITS   (_SVIN_CHARACTER_BYTES/32)

//using VDP2 VRAM static allocation with everything contingent
// Bank 0
//  0x00000000 - 0x0001FFFF NBG0 character pattern name data topleft 512x256 (32x16*16x16) = 0x20000
// Bank 1
//  0x00020000 - 0x00037FFF NBG0 character pattern name data botleft 512x192 (32x12*16x16) = 0x18000
//  0x00038000 - 0x0003F7FF NBG0 character pattern name data topright 192x160 (12x10*16x16) = 0x7800
//  0x0003F800 - 0x0003FFFF NBG0 character pattern name data midright 128x16 (8x1*16x16) = 0x800
// Bank 2
//  0x00040000 - 0x000403FF NBG0 character pattern name data midright 64x16 (4x1*16x16) = 0x400
//  0x00040400 - 0x0004D000 NBG0 character pattern name data bottomright 192*272 (12x17*16x16) = 0xCC00
//  0x00050000 - 0x00051FFF NBG0 pattern name data 64x32*4 = 0x2000
//  0x00052000 - 0x00053FFF NBG1 pattern name data 64x32*4 = 0x2000
// Bank 3
//  0x00060000 - 0x0006B3FF NBG1 character pattern name data bottomright 576x80 (36x5*16x16) = 0xB400
//  0x0007F000 - 0x0007FFFF NBG1 character pattern name data specials

#define _SVIN_NBG0_CHPNDR_START (VDP2_VRAM_ADDR(0,0))
#define _SVIN_NBG0_CHPNDR_SIZE (_SVIN_SCREEN_WIDTH*_SVIN_SCREEN_HEIGHT)
#define _SVIN_NBG0_PNDR_START (VDP2_VRAM_ADDR(2,0x10000))
#define _SVIN_NBG0_PNDR_SIZE (64*32*4)
#define _SVIN_NBG1_CHPNDR_START (VDP2_VRAM_ADDR(3,0))
#define _SVIN_NBG1_CHPNDR_SIZE (576*80)
#define _SVIN_NBG1_PNDR_START (VDP2_VRAM_ADDR(2,0x10000) + _SVIN_NBG0_PNDR_SIZE)
#define _SVIN_NBG1_PNDR_SIZE (_SVIN_NBG0_PNDR_SIZE)
#define _SVIN_NBG1_CHPNDR_SPECIALS_ADDR (VDP2_VRAM_ADDR(3,0x1F000))
#define _SVIN_NBG1_CHPNDR_SPECIALS_INDEX ((0x7F000)/32)

void _svin_init();
void _svin_delay(int milliseconds);
void _svin_background_set(int starting_fad,int starting_fad_palette);
void _svin_background_clear();
void _svin_background_fade_to_black();

#endif
