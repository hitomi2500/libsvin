#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"
#include "cd-block_multiread.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)

#define _SVIN_SPRITE_TILES_WIDTH 29
#define _SVIN_SPRITE_TILES_HEIGTH 56
#define _SVIN_SPRITE_TILES (_SVIN_SPRITE_TILES_WIDTH*_SVIN_SPRITE_TILES_HEIGTH)

int iLastIndex[2];

void 
_svin_sprite_init()
{
    char * p = (char*)_SVIN_SPRITE_NBG0_GLOBAL_USAGE_ADDR;
    memset(p,0,4096);
    p = (char*)_SVIN_SPRITE_NBG1_GLOBAL_USAGE_ADDR;
    memset(p,0,2560);
    iLastIndex[0] = 0;
    iLastIndex[1] = 0;
}

void 
_svin_sprite_clear(int iPosition)
{
    int * p32;
    int * p32a;
    int x,y;
    //clear the previous image
    _svin_set_cycle_patterns_cpu();
    switch (iPosition)
    {
        case 0:
            p32 = (int*)_SVIN_NBG0_PNDR_START;
            p32a = (int*)_SVIN_NBG1_PNDR_START;
            for (y=0;y<_SVIN_SPRITE_TILES_HEIGTH;y++)
                for (x=0;x<_SVIN_SPRITE_TILES_WIDTH;x++)
                {
                    p32[y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    //not killing textbox, lines 44 thru 53
                    if ((y<44) || (y>53))
                        p32a[y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                }
            break;
        case 1:
            p32 = (int*)_SVIN_NBG0_PNDR_START;
            p32a = (int*)_SVIN_NBG1_PNDR_START;
            for (y=0;y<_SVIN_SPRITE_TILES_HEIGTH;y++)
                for (x=_SVIN_SPRITE_TILES_WIDTH;x<_SVIN_SPRITE_TILES_WIDTH*2;x++)
                {
                    p32[y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    //not killing textbox, lines 44 thru 53
                    if ((y<44) || (y>53))
                        p32a[y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                }
            break;
        case 2:
            p32 = (int*)_SVIN_NBG0_PNDR_START;
            p32a = (int*)_SVIN_NBG1_PNDR_START;
            for (y=0;y<_SVIN_SPRITE_TILES_HEIGTH;y++)
                for (x=_SVIN_SPRITE_TILES_WIDTH*2;x<64;x++)
                {
                    p32[y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    //not killing textbox, lines 44 thru 53
                    if ((y<44) || (y>53))
                        p32a[y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                }
            for (y=64;y<64+_SVIN_SPRITE_TILES_HEIGTH;y++)
                for (x=0;x<(64-_SVIN_SPRITE_TILES_WIDTH*2);x++)
                {
                    p32[y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    //not killing textbox, lines 44 thru 53
                    if ((y<44+64) || (y>53+64))
                        p32a[y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                }
            break;
    }
    //_svin_set_cycle_patterns_nbg();
}

void 
_svin_sprite_draw(char * filename, int iLayer, int iPosition)
{
    uint8_t * big_buffer;
    int i,x,y;
    int iPointer;
    //int iTile;
    char * pGlobalUsage[2];
    int iFree;
    char c;
    int * p32;
    bool bFound;
    int iFound;
    int iSize, iSize_Fixed;
    int iOffset;
    int iTilesNumberForLayer;

    //first let's find sprite FAD
    fad_t _sprite_fad;
    assert(true == _svin_filelist_search(filename,&_sprite_fad,&iSize));
    iSize_Fixed = ((iSize/2048)+1)*2048;

    big_buffer = malloc(iSize_Fixed);

    pGlobalUsage[0] = (char*)_SVIN_SPRITE_NBG0_GLOBAL_USAGE_ADDR;
    pGlobalUsage[1] = (char*)_SVIN_SPRITE_NBG1_GLOBAL_USAGE_ADDR;

    int iLayer_fixed = iLayer;
    if (iLayer == 2)
        iLayer_fixed = 1; //for layer 2 using NBG1 as well

    switch(iLayer_fixed)
    {
        case 0:
            iTilesNumberForLayer = _SVIN_NBG0_CHPNDR_SIZE/_SVIN_CHARACTER_BYTES;
            break;
        case 1:
            iTilesNumberForLayer = _SVIN_NBG1_CHPNDR_SIZE/_SVIN_CHARACTER_BYTES;
            break;
        case 2:
            iTilesNumberForLayer = _SVIN_NBG2_CHPNDR_SIZE/_SVIN_CHARACTER_BYTES;
            break;
        default:
            iTilesNumberForLayer = 0;
    }
    
    assert (iTilesNumberForLayer > 0);

    //reading whole file at once
    cd_block_multiple_sectors_read(_sprite_fad, iSize_Fixed/2048, big_buffer);

    //calculating tiles number
    int iTilesNumber = 0;
    for (i=0;i<_SVIN_SPRITE_TILES;i++)
    {
        if (big_buffer[i])
            iTilesNumber++;
    }

    int iLastIndex_to_free = iLastIndex[iLayer_fixed];

    iFree = 0;
    while (iFree < iTilesNumber)
    {
        iLastIndex_to_free++;
        if (iLastIndex_to_free > 255) iLastIndex_to_free = 1;
        for (i=0;i<iTilesNumberForLayer;i++)
        {
            c = pGlobalUsage[iLayer_fixed][i];
            if (c==iLastIndex_to_free){
                pGlobalUsage[iLayer_fixed][i] = 0; //freeing
            }
        }
        //recalculate free
        iFree = 0;
        for (i=0;i<iTilesNumberForLayer;i++)
        {
            c = pGlobalUsage[iLayer_fixed][i];
            if (c==0){
                iFree++;
            }
        }

    }

    iPointer = _SVIN_SPRITE_TILES; //position within buffer

    //VRAM available, fill it
    //choose next fill index
    iLastIndex[iLayer_fixed]++;
    if (iLastIndex[iLayer_fixed] > 255)
        iLastIndex[iLayer_fixed] = 1;
        
    //fill data

    iFound = 0;

    _svin_set_cycle_patterns_cpu();

    int x_start=0,x_end=0;
    switch (iPosition)
    {
        case 0:
            x_start = 0;
            x_end = _SVIN_SPRITE_TILES_WIDTH;
            break;
        case 1:
            x_start = _SVIN_SPRITE_TILES_WIDTH;
            x_end = _SVIN_SPRITE_TILES_WIDTH*2;
            break;
        case 2:
            x_start = _SVIN_SPRITE_TILES_WIDTH*2;
            x_end = 64;
            break;
    }

    for (y=0;y<_SVIN_SPRITE_TILES_HEIGTH;y++)
    {
        for (x=x_start;x<x_end;x++)
        {
            if (big_buffer[y*_SVIN_SPRITE_TILES_WIDTH+x%_SVIN_SPRITE_TILES_WIDTH])
            {
                //searching first free tile data slot
                bFound = false;    
                for (i=0;(i<iTilesNumberForLayer)&&(bFound == false);i++)
                {
                    if (0 == pGlobalUsage[iLayer_fixed][i]) {
                        bFound = true;
                        iFound = i;
                        pGlobalUsage[iLayer_fixed][i] = iLastIndex[iLayer_fixed];
                    }
                }
                //copying tile data
                if (iLayer_fixed == 0)
                {
                    memcpy((char*)(_SVIN_NBG0_CHPNDR_START+iFound*64),big_buffer+iPointer,64);
                    p32 = (int*)_SVIN_NBG0_PNDR_START;
                }
                else
                {
                    memcpy((char*)(_SVIN_NBG1_CHPNDR_START+iFound*64),big_buffer+iPointer,64);
                    p32 = (int*)_SVIN_NBG1_PNDR_START;
                }
                //setting tile index
                if (iLayer_fixed == 0)
                {
                    iOffset = (_SVIN_NBG0_CHPNDR_START - VDP2_VRAM_ADDR(0,0))/32;
                    p32[y*64+x] = 0x00000000 | 0x100000*(1+iLayer_fixed*3+iPosition) | (iOffset + iFound*2); //palette 0, transparency on
                }
                else
                {
                    //not killing textbox, lines 44 thru 53
                    if ((y<44) || (y>53))
                    {
                        iOffset = (_SVIN_NBG1_CHPNDR_START - VDP2_VRAM_ADDR(0,0))/32;
                        p32[y*64+x] = 0x00000000 | 0x100000*(1+iLayer_fixed*3+iPosition) | (iOffset + iFound*2); //palette 0, transparency on
                    }
                }
                //moving pointer
                iPointer+=64;
            }
        }
    }

    //load palette
    //using palettes 1 thru 6
    if (iLayer == 0)
    {
        _svin_background_set_palette(1+iLayer_fixed*3+iPosition,big_buffer+iPointer);
    }
    else if  (iLayer == 1)
    {
        _svin_background_set_palette_half_lo(1+iLayer_fixed*3+iPosition,big_buffer+iPointer);
    }
    else //palette 2
    {
        _svin_background_set_palette_half_hi(1+iLayer_fixed*3+iPosition,big_buffer+iPointer);
    }
    //_svin_set_cycle_patterns_nbg();

    free(big_buffer);
}