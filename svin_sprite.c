#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)

void 
_svin_sprite_init()
{
    char * p = (char*)_SVIN_SPRITE_NBG0_GLOBAL_USAGE_ADDR;
    memset(p,0,4096);
    p = (char*)_SVIN_SPRITE_NBG1_GLOBAL_USAGE_ADDR;
    memset(p,0,2048);
}

void 
_svin_sprite_draw(char * filename, int iLayer, int iPosition)
{
    uint8_t * usage_buffer = malloc(2048);
    uint8_t * tmp_buffer = malloc(2048);
    uint8_t * tmp_buffer2 = malloc(2048);
    //uint8_t * tile_buffer = malloc(2048);
    //char * pDebug = (char*)0x20200000;
    int i,x,y;
    int iPointer;
    //int iTile;
    char * pGlobalUsage[2];
    int iFree;
    int iLastIndex;
    char c;
    int * p32;
    bool bFound;
    int iFound;

    pGlobalUsage[0] = (char*)_SVIN_SPRITE_NBG0_GLOBAL_USAGE_ADDR;
    pGlobalUsage[1] = (char*)_SVIN_SPRITE_NBG1_GLOBAL_USAGE_ADDR;

    int iLayer_fixed = iLayer;
    if (iLayer == 2)
        iLayer_fixed = 1; //for layer 2 using NBG1 as well

    //first let's find sprite FAD
    int _sprite_fad = _svin_filelist_search(filename);
    //strcpy(pDebug,filename);
    assert(_sprite_fad > 0);

    //loading map usage
    cd_block_sector_read(_sprite_fad, tmp_buffer);
    _sprite_fad++;
    memcpy(usage_buffer,tmp_buffer,1960);//35*56 tiles = 280 * 448
    //calculating tiles number
    int iTilesNumber = 0;
    for (i=0;i<1960;i++)
    {
        if (usage_buffer[i])
            iTilesNumber++;
    }
    //verifying if vram have enough free data
    iFree = 0;
    iLastIndex = 0;
    for (i=0;i<(4096-iLayer_fixed*2048);i++)
    {
        c = pGlobalUsage[iLayer_fixed][i];
        if (c==0){
            iFree++;
        }
        else{
            if (iLastIndex < c)
                iLastIndex = c;
        }
    }

    assert(iFree > iTilesNumber); //TODO: VRAM re-allocation and management, now just give up if VRAM is full

    iPointer = 1960; //position within buffer

    //VRAM available, fill it
    //choose next fill index
    if (iLastIndex > 254)
        iLastIndex = 1;
    else
        iLastIndex++;
    //fill data
    
    //iTile = 0;
    iFound = 0;

    _svin_set_cycle_patterns_cpu();

    for (y=0;y<56;y++)
    {
        for (x=0;x<35;x++)
        {
            if (usage_buffer[y*35+x])
            {
                //searching first free tile data slot
                bFound = false;    
                for (i=0;(i<(4096-iLayer_fixed*2048))&&(bFound == false);i++)
                {
                    if (0 == pGlobalUsage[iLayer_fixed][i]) {
                        bFound = true;
                        iFound = i;
                        pGlobalUsage[iLayer_fixed][i] = iLastIndex;
                    }
                }
                //do we need to load data?
                if (iPointer < (2048-64))
                {
                    //copying tile data
                    //no loading required, use mem
                    if (iLayer_fixed == 0)
                    {
                        memcpy((char*)(_SVIN_NBG0_CHPNDR_START+iFound*64),tmp_buffer+iPointer,64);
                        p32 = (int*)_SVIN_NBG0_PNDR_START;
                    }
                    else
                    {
                        memcpy((char*)(_SVIN_NBG1_CHPNDR_START+iFound*64),tmp_buffer+iPointer,64);
                        p32 = (int*)_SVIN_NBG1_PNDR_START;
                    }
                }
                else
                {
                    //loading required, copy first part
                    int part = 2048 - iPointer;

                    if (iLayer_fixed == 0)
                    {
                        memcpy((char*)(_SVIN_NBG0_CHPNDR_START+iFound*64),tmp_buffer+iPointer,part);
                        p32 = (int*)_SVIN_NBG0_PNDR_START;
                    }
                    else
                    {
                        memcpy((char*)(_SVIN_NBG1_CHPNDR_START+iFound*64),tmp_buffer+iPointer,part);
                        p32 = (int*)_SVIN_NBG1_PNDR_START;
                    }

                    //load next piece
                    cd_block_sector_read(_sprite_fad, tmp_buffer);
                    _sprite_fad++;

                    if (iLayer_fixed == 0)
                    {
                        memcpy((char*)(_SVIN_NBG0_CHPNDR_START+iFound*64+part),tmp_buffer,64-part);
                    }
                    else
                    {
                        memcpy((char*)(_SVIN_NBG1_CHPNDR_START+iFound*64+part),tmp_buffer,64-part);
                    }
                    iPointer -= 2048;
                }
                //setting tile index
                if (iLayer_fixed == 0)
                {
                    p32[y*64+x+iPosition] = 0x00000000 | 0x100000*(1+iLayer_fixed*3+iPosition) | iFound*2; //palette 0, transparency on
                }
                else
                {
                    //not killing textbox, lines 44 thru 53
                    if ((y<44) || (y>53))
                        p32[y*64+x+iPosition] = 0x00000000 | 0x100000*(1+iLayer_fixed*3+iPosition) | 0x2800 | iFound*2; //palette 0, transparency on
                }
                //moving pointer
                iPointer+=64;
            }
        }
    }

    _svin_set_cycle_patterns_nbg();

    //load palette
    memcpy(tmp_buffer2,tmp_buffer+iPointer,2048-iPointer);
    if (2048-iPointer < 768)
    {
        cd_block_sector_read(_sprite_fad, tmp_buffer);
        memcpy(tmp_buffer2+2048-iPointer,tmp_buffer,256*3+iPointer-2048);
    }
    //using palettes 1 thru 6
    if (iLayer == 0)
    {
        _svin_background_set_palette(1+iLayer_fixed*3+iPosition,tmp_buffer2);
    }
    else if  (iLayer == 1)
    {
        _svin_background_set_palette_half_lo(1+iLayer_fixed*3+iPosition,tmp_buffer2);
    }
    else //palette 2
    {
        _svin_background_set_palette_half_hi(1+iLayer_fixed*3+iPosition,tmp_buffer2);
    }

    //free(tile_buffer);
    free(tmp_buffer2);
    free(tmp_buffer);
    free(usage_buffer); 
}