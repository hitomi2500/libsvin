#ifndef _SVIN_SPRITE_H_
#define _SVIN_SPRITE_H_

#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

typedef struct {
    char filename[252];
	uint8_t position;
    uint8_t status;
    uint8_t age;
    uint16_t size_x;
    uint16_t size_y;
    char usage[384];
    uint8_t palette[768];
} _svin_sprite_cache_entry_t;

//there are 3 tables in VDP2 sprite cache
//
//first table includes VDP2 tiles data for sprites (NBG0 and NBG1), 
//  size is (0x18000+0x18000+0x8000+0x10000)/64 = 4608 entries, reserving 8192 bytes, 1 byte/tile
#define SVIN_SPRITE_CACHE_TILES_SIZE 8192
//
//second table include VDP2 tiles names for sprites (NBG0 and NBG1), 
//  size is (0x8000+0x8000)/4 = 16384 entries, reserving 2048 bytes, 1 byte/tile
#define SVIN_SPRITE_CACHE_NAMES_SIZE 16384
//
//third table include all sprites the tiles correspond to with IDs and names
#define SVIN_SPRITE_CACHE_SPRITES_SIZE 64

//statuses fro sprites
#define SVIN_SPRITE_CACHE_UNUSED 0
#define SVIN_SPRITE_CACHE_LOADED 1
#define SVIN_SPRITE_CACHE_SHOWN 2

void _svin_sprite_init();
void _svin_sprite_clear(int iPosition);
void _svin_sprite_draw(char * filename, int iLayer, int iPosition, int iPalette);

#endif
