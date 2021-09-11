#ifndef _SVIN_SPRITE_H_
#define _SVIN_SPRITE_H_

#define _SVIN_SPRITE_NBG0_GLOBAL_USAGE_ADDR 0x202F0000
#define _SVIN_SPRITE_NBG1_GLOBAL_USAGE_ADDR 0x202F2000

#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

void _svin_sprite_init();
void _svin_sprite_clear(int iPosition);
void _svin_sprite_draw(char * filename, int iLayer, int iPosition);

#endif
