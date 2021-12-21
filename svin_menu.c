#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"
#include "svin_text.h"
#include "svin_menu.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)

_svin_menu_item_type _svin_menu_items[10];
uint8_t _svin_menu_items_count;

void 
_svin_menu_init()
{
    int *_pointer32;

    _svin_set_cycle_patterns_cpu();

    //-------------- setup pattern names -------------------
    _pointer32 = (int *)_SVIN_NBG2_PNDR_START;

    //menu is 40x20 tiles, to fit into a single plane, and into the same tilespace as dialog 
    int index = 0;
    int iOffset;
    for (int y = 25; y < 45; y++)
    {
        //plane 0 only
        iOffset = y * 64;
        for (int x = 24; x < 64; x++)
        {
            _pointer32[iOffset + x] = 0x10700000 + _SVIN_NBG2_CHPNDR_TEXTBOX_INDEX + _SVIN_CHARACTER_UNITS * index; //palette 7, transparency on
            index++;
        }
    }

    _svin_set_cycle_patterns_nbg();

    _svin_menu_clear();

    _svin_menu_items_count = 0; //starting unpopulated
}

void 
_svin_menu_clear()
{
    _svin_set_cycle_patterns_cpu();

    int * _pointer32 = (int *)(_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR);
    for (unsigned int i = 0; i < (_SVIN_NBG2_CHPNDR_TEXTBOX_SIZE) / sizeof(int); i+=4)
    {
        _pointer32[i] = 0x0F000F00;
        _pointer32[i+1] = 0x0F000F00;
        _pointer32[i+2] = 0x000F000F;
        _pointer32[i+3] = 0x000F000F;
    }

    _svin_set_cycle_patterns_nbg();
}

void 
_svin_menu_disable()
{
    //filling entire textbox range with transparent color 0
    memset((void*)_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR,0,_SVIN_NBG2_CHPNDR_TEXTBOX_SIZE);
}

void 
_svin_menu_populate(int jump, const char * text)
{
        //just fill internal structures, do not draw anything yet
        if (_svin_menu_items_count >= 10) return;
        _svin_menu_items[_svin_menu_items_count].id = _svin_menu_items_count;
        _svin_menu_items[_svin_menu_items_count].jump = jump;
        _svin_menu_items[_svin_menu_items_count].line = malloc(2048);
        strcpy(_svin_menu_items[_svin_menu_items_count].line,text);
        _svin_menu_items_count++;
}

int 
_svin_menu_activate()
{
        return 0;
}
