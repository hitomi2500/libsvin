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
    memset(p,4096);
    char * p = (char*)_SVIN_SPRITE_NBG1_GLOBAL_USAGE_ADDR;
    memset(p,2048);
}

void 
_svin_sprite_draw(char * filename, int iLayer, int iPosition)
{
    uint8_t * usage_buffer = malloc(2048);
    uint8_t * tmp_buffer = malloc(2048);
    uint8_t * tile_buffer = malloc(2048);
    bool bFinished = false;
    char * pDebug = (char*)0x20200000;
    int i,j,k,x,y;
    int iPointer = 0;
    int iTile = 0;
    char * pGlobalUsage[2];

    pGlobalUsage[0] = (char*)_SVIN_SPRITE_NBG0_GLOBAL_USAGE_ADDR;
    pGlobalUsage[1] = (char*)_SVIN_SPRITE_NBG1_GLOBAL_USAGE_ADDR;

    //first let's find sprite FAD
    int _sprite_fad = _svin_filelist_search(filename):
    assert(_sprite_fad > 0);

    //loading map usage
    cd_block_sector_read(_sprite_fad, tmp_buffer);
    memcpy(usage_buffer,tmp_buffer,1960);//35*56 tiles = 280 * 448
    //calculating tiles number
    int iTilesNumber = 0;
    for (i=0;i<1960;i++)
    {
        if (usage_buffer[i])
            iTilesNumber++;
    }
    //searching within a global usage table for a free spot
    bool bFound = false;
    int iFound = 0;
    for (i=0;i<(4096-iLayer*2048),bFound == false,i++)
    {
        if (pGlobalUsage[iLayer][i]==0)
        {
            bFound = true;
            iFound = i;
        }
    }

    #error here, VRAM tiles doesn't need to be consecutive. should check free VRAM instead

    if (false == bFound)
        return; //TODO: VRAM re-allocation and management, now just give up if VRAM is full

    pGlobalUsage[iLayer]iFound] = 1; //marking as used

    iPointer = 1960; //position within buffer
    iTile = 0;
    for (y=0;y<56;y++)
    {
        for (x=0;x<35;x++)
        {
            if (usage_buffer[i])
            {
                //this tile is used, load it
                if (iPointer < (2048-64))
                {
                    //no loading required, use mem
                    if (iLayer == 0)
                        memcpy(tmp_buffer+iPointer, _SVIN_NBG0_CHPNDR_START+iFound,64);
                    else
                    iPointer+=64;
                else
                {

                    //loading required, use mem
                    //memcpy(tmp_buffer+iPointer, VDP2_VRAM_ADDR(y*),64);
                }
            }
            iTile++;
        }
    }

    free(tile_buffer);
    free(tmp_buffer);
    free(usage_buffer);
    }   
}