#ifndef _SVIN_SPRITE_H_
#define _SVIN_SPRITE_H_

#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

typedef struct {
    const char *filename;
    bool valid;
} _svin_sprite_t;

#define SVIN_SPRITE_CACHE_SIZE 256

void _svin_sprite_init();
void _svin_sprite_clear(int iPosition);
void _svin_sprite_draw(char * filename, int iLayer, int iPosition, int iPalette);

#endif
