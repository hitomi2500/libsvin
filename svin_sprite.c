#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)

#define _SVIN_SPRITE_TILES_WIDTH 44
#define _SVIN_SPRITE_TILES_HEIGTH 56

int iLastIndex[3];

char * _svin_sprite_NBG0_usage;
char * _svin_sprite_NBG1_usage;
char * _svin_sprite_NBG2_usage;
char * _svin_sprite_cache_tiles;
char * _svin_sprite_cache_names;
_svin_sprite_cache_entry_t * _svin_sprite_cache_sprites;
int iNBG0_Free_Tiles;
int iNBG1_Free_Tiles;

void 
_svin_sprite_init()
{
    _svin_sprite_NBG0_usage = malloc(_SVIN_NBG0_CHPNDR_SIZE/_SVIN_CHARACTER_BYTES);
    memset(_svin_sprite_NBG0_usage,0,_SVIN_NBG0_CHPNDR_SIZE/_SVIN_CHARACTER_BYTES);
    _svin_sprite_NBG1_usage = malloc(_SVIN_NBG1_CHPNDR_SIZE/_SVIN_CHARACTER_BYTES);
    memset(_svin_sprite_NBG1_usage,0,_SVIN_NBG1_CHPNDR_SIZE/_SVIN_CHARACTER_BYTES);
    _svin_sprite_NBG2_usage = malloc(_SVIN_NBG2_CHPNDR_SIZE/_SVIN_CHARACTER_BYTES);
    memset(_svin_sprite_NBG2_usage,0,_SVIN_NBG2_CHPNDR_SIZE/_SVIN_CHARACTER_BYTES);
    iLastIndex[0] = 0;
    iLastIndex[1] = 0;
    iLastIndex[2] = 0;
	iNBG0_Free_Tiles = 3072;
	iNBG1_Free_Tiles = 1536;
	//allocate cache tiles
    _svin_sprite_cache_tiles = malloc(6144);//more than SVIN_SPRITE_CACHE_TILES_SIZE because of picky malloc
	//mark all tiles as unused (zero id)
	for (int i=0;i<SVIN_SPRITE_CACHE_TILES_SIZE;i++)
		_svin_sprite_cache_tiles[i] = 0;
	//allocate cache names
    _svin_sprite_cache_names = malloc(SVIN_SPRITE_CACHE_NAMES_SIZE);
	//mark all names as unused (zero id)
	for (int i=0;i<SVIN_SPRITE_CACHE_NAMES_SIZE;i++)
		_svin_sprite_cache_names[i] = 0;
	//allocate sprite cache
    _svin_sprite_cache_sprites = _svin_alloc_lwram(sizeof(_svin_sprite_cache_entry_t)*SVIN_SPRITE_CACHE_SPRITES_SIZE,0x20280000);
	//mark all sprites as unused
	for (int i=0;i<SVIN_SPRITE_CACHE_SPRITES_SIZE;i++)
		_svin_sprite_cache_sprites[i].status = SVIN_SPRITE_CACHE_UNUSED;
}

void 
_svin_sprite_clear(int iPosition)
{
    int * p32[3];
    //int x,y;
	int i;
    //clear the previous image
    _svin_set_cycle_patterns_cpu();

    p32[0] = (int*)_SVIN_NBG0_PNDR_START;
    p32[1] = (int*)_SVIN_NBG1_PNDR_START;
    p32[2] = (int*)_SVIN_NBG2_PNDR_START;
	
	/*//searching for all sprites in the specified position
	for (i=0;i<SVIN_SPRITE_CACHE_SPRITES_SIZE;i++)
	{
		if ( (_svin_sprite_cache_sprites[i].status == SVIN_SPRITE_CACHE_SHOWN) && (_svin_sprite_cache_sprites[i].position == iPosition) )
		{
			//found shown sprite at current position, clearing it from names, but leaving in tiles and in sprites
			for (int j=0; j < SVIN_SPRITE_CACHE_NAMES_SIZE; j++)
			{
				if (_svin_sprite_cache_names[j] == i)
				{
					//found a match in names, clearing it
					_svin_sprite_cache_names[j] = 0; //zero id, i.e. unused
					//now clean the actual tables values in VDP2
					if (j < SVIN_SPRITE_CACHE_NAMES_SIZE/2)
						p32[0][j] = 0x10000000 + _SVIN_NBG0_CHPNDR_SPECIALS_INDEX;
					else
						p32[1][j-SVIN_SPRITE_CACHE_NAMES_SIZE/2] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX;
				}
			}
		}
	}*/
		
	
    switch (iPosition)
    {
        case 0:
            //for left 
            for (y=0;y<_SVIN_SPRITE_TILES_HEIGTH;y++)
                for (x=0;x<_SVIN_SPRITE_TILES_WIDTH;x++)
                {
                    p32[0][y*64+x] = 0x10000000 + _SVIN_NBG0_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    p32[1][y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    //not killing textbox, lines 44 thru 53
                    if ((y<44) || (y>53))
                        p32[2][y*64+x] = 0x10000000 + _SVIN_NBG2_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                }
            break;
        case 1:
            //for mid 
            for (y=0;y<_SVIN_SPRITE_TILES_HEIGTH;y++)
                for (x=_SVIN_SPRITE_TILES_WIDTH/2;x<64;x++)
                {
                    p32[0][y*64+x] = 0x10000000 + _SVIN_NBG0_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    p32[1][y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    //not killing textbox, lines 44 thru 53
                    if ((y<44) || (y>53))
                        p32[2][y*64+x] = 0x10000000 + _SVIN_NBG2_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                }
            break;
            for (y=64;y<64+_SVIN_SPRITE_TILES_HEIGTH;y++)
                for (x=0;x<(_SVIN_SPRITE_TILES_WIDTH*3/4) - 64;x++)
                {
                    p32[0][y*64+x] = 0x10000000 + _SVIN_NBG0_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    p32[1][y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    //not killing textbox, lines 44 thru 53
                    if ((y<44) || (y>53))
                        p32[2][y*64+x] = 0x10000000 + _SVIN_NBG2_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                }
            break;
        case 2:
            //for right 
            for (y=0;y<_SVIN_SPRITE_TILES_HEIGTH;y++)
                for (x=_SVIN_SPRITE_TILES_WIDTH;x<64;x++)
                {
                    p32[0][y*64+x] = 0x10000000 + _SVIN_NBG0_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    p32[1][y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    //not killing textbox, lines 44 thru 53
                    if ((y<44) || (y>53))
                        p32[2][y*64+x] = 0x10000000 + _SVIN_NBG2_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                }
            for (y=64;y<64+_SVIN_SPRITE_TILES_HEIGTH;y++)
                for (x=0;x<_SVIN_SPRITE_TILES_WIDTH*2-64;x++)
                {
                    p32[0][y*64+x] = 0x10000000 + _SVIN_NBG0_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    p32[1][y*64+x] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                    //not killing textbox, lines 44 thru 53
                    if ((y<44+64) || (y>53+64))
                        p32[2][y*64+x] = 0x10000000 + _SVIN_NBG2_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
                }
            break;
    }
    _svin_set_cycle_patterns_nbg();
}

void 
_svin_sprite_draw(char * filename, int iLayer, int iPosition, int iPalette)
{
    uint8_t * big_buffer;
    int i,x,y;
    int iPointer;
    char * pGlobalUsage[3];
    int iFree;
    char c;
    int * p32;
    bool bFound;
    int iFound;
    int iSize, iSize_Fixed;
    int iOffset;
    int iTilesNumberForLayer;
    uint8_t iSizeX,iSizeY;
	
	switch(iLayer)
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
	
	//using palettes 1 thru 6
    int iPaletteIndex=1;
    switch (iPosition)
    {
        case 0:
            //left, only layers NBG0 and NBG1
            iPaletteIndex = 1+iLayer;
            break;
        case 1:
            //mid, only layers NBG1 and NBG2
            iPaletteIndex = 3+iLayer;
            break;
        case 2:
            //right, only layers NBG0 and NBG1
            iPaletteIndex = 5+iLayer;
            break;
    }
	
	assert (iPaletteIndex > 0);
    assert (iPaletteIndex < 7);
	
	int x_start=0,x_end=0;
	
	//searching for sprite in the cache
	/*for (i=0;i<SVIN_SPRITE_CACHE_SPRITES_SIZE;i++)
	{
		if ( (_svin_sprite_cache_sprites[i].status != SVIN_SPRITE_CACHE_UNUSED) && (0==strcmp(_svin_sprite_cache_sprites[i].filename,filename)) )
		{
            switch (iPosition)
            {
                case 0:
                    x_start = 0;
                    break;
                case 1:
                    x_start = _SVIN_SPRITE_TILES_WIDTH - (_svin_sprite_cache_sprites[i].size_x/2);
                    break;
                case 2:
                    x_start = _SVIN_SPRITE_TILES_WIDTH*2 - _svin_sprite_cache_sprites[i].size_x;
                    break;
            }
			x_end = x_start + _svin_sprite_cache_sprites[i].size_x;
			//found sprite for this filename in VRAM, not loading anything, just using existing VRAM data
			//sprites in VRAM are sorted by address, even if fragmented, using this feature.
			for (y=0;y<_svin_sprite_cache_sprites[i].size_y;y++)
			{
				for (x=x_start;x<x_end;x++)
				{
					if (big_buffer[y*iSizeX+(x-x_start)+2])
					{
						//searching first free tile data slot
						bFound = false;    
						for (i=0;(i<iTilesNumberForLayer)&&(bFound == false);i++)
						{
							if (0 == pGlobalUsage[iLayer][i]) {
								bFound = true;
								iFound = i;
								pGlobalUsage[iLayer][i] = iLastIndex[iLayer];
							}
						}
						//copying tile data
						switch (iLayer)
						{
							case 0:
								memcpy((char*)(_SVIN_NBG0_CHPNDR_START+iFound*64),big_buffer+iPointer,64);
								p32 = (int*)_SVIN_NBG0_PNDR_START;
								break;
							case 1:
								memcpy((char*)(_SVIN_NBG1_CHPNDR_START+iFound*64),big_buffer+iPointer,64);
								p32 = (int*)_SVIN_NBG1_PNDR_START;
								break;
							case 2:
								memcpy((char*)(_SVIN_NBG2_CHPNDR_START+iFound*64),big_buffer+iPointer,64);
								p32 = (int*)_SVIN_NBG2_PNDR_START;
								break; 
						}
						//setting tile index
						switch (iLayer)
						{
							case 0:
								iOffset = (_SVIN_NBG0_CHPNDR_START - VDP2_VRAM_ADDR(0,0))/32;
								if (x<64)
									p32[y*64+x] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
								else
									p32[(y+64)*64+x-64] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
								break;
							case 1:
								//not killing textbox, lines 44 thru 53
								//if ((y<44) || (y>53))
								{
									iOffset = (_SVIN_NBG1_CHPNDR_START - VDP2_VRAM_ADDR(0,0))/32;
									if (x<64)
										p32[y*64+x] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
									else
										p32[(y+64)*64+x-64] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
								}
								break;
							case 2:
								//not killing textbox, lines 44 thru 53
								if ((y<44) || (y>53))
								{
									iOffset = (_SVIN_NBG2_CHPNDR_START - VDP2_VRAM_ADDR(0,0))/32;
									if (x<64)
										p32[y*64+x] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
									else
										p32[(y+64)*64+x-64] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
								}
								break; 
						}
						//moving pointer
						iPointer+=64;
					}
				}
			}
		}
	}*/

    //first let's find sprite FAD
    fad_t _sprite_fad;
    assert(true == _svin_filelist_search(filename,&_sprite_fad,&iSize));
    iSize_Fixed = ((iSize/2048)+1)*2048;

    big_buffer = malloc(iSize_Fixed);

    pGlobalUsage[0] = _svin_sprite_NBG0_usage;
    pGlobalUsage[1] = _svin_sprite_NBG1_usage;
    pGlobalUsage[2] = _svin_sprite_NBG2_usage;

    //reading whole file at once
    _svin_cd_block_sectors_read(_sprite_fad, big_buffer, iSize_Fixed);

    //getting sizes in tiles
    iSizeX = big_buffer[0];
    iSizeY = big_buffer[1];

    assert (iSizeY == _SVIN_SPRITE_TILES_HEIGTH);
    //assert (iSizeX <= _SVIN_SPRITE_TILES_WIDTH);

                switch (iPosition)
            {
                case 0:
                    x_start = 0;
                    break;
                case 1:
                    x_start = _SVIN_SPRITE_TILES_WIDTH - (iSizeX/2);
                    break;
                case 2:
                    x_start = _SVIN_SPRITE_TILES_WIDTH*2 -iSizeX;
                    break;
            }

    //calculating tiles number
    int iTilesNumber = 0;
    for (i=0;i<iSizeX*iSizeY;i++)
    {
        if (big_buffer[i+2])
            iTilesNumber++;
    }

    int iLastIndex_to_free = iLastIndex[iLayer];

    iFree = 0;
    while (iFree < iTilesNumber)
    {
        iLastIndex_to_free++;
        if (iLastIndex_to_free > 255) iLastIndex_to_free = 1;
        for (i=0;i<iTilesNumberForLayer;i++)
        {
            c = pGlobalUsage[iLayer][i];
            if (c==iLastIndex_to_free){
                pGlobalUsage[iLayer][i] = 0; //freeing
            }
        }
        //recalculate free
        iFree = 0;
        for (i=0;i<iTilesNumberForLayer;i++)
        {
            c = pGlobalUsage[iLayer][i];
            if (c==0){
                iFree++;
            }
        }

    }

    iPointer = iSizeX*iSizeY+2; //position within buffer

    //VRAM available, fill it
    //choose next fill index
    iLastIndex[iLayer]++;
    if (iLastIndex[iLayer] > 255)
        iLastIndex[iLayer] = 1;
        
    //fill data

    iFound = 0;

    _svin_set_cycle_patterns_cpu();


    x_end = x_start + iSizeX;

    for (y=0;y<iSizeY;y++)
    {
        for (x=x_start;x<x_end;x++)
        {
            if (big_buffer[y*iSizeX+(x-x_start)+2])
            {
                //searching first free tile data slot
                bFound = false;    
                for (i=0;(i<iTilesNumberForLayer)&&(bFound == false);i++)
                {
                    if (0 == pGlobalUsage[iLayer][i]) {
                        bFound = true;
                        iFound = i;
                        pGlobalUsage[iLayer][i] = iLastIndex[iLayer];
                    }
                }
                //copying tile data
                switch (iLayer)
                {
                    case 0:
                        memcpy((char*)(_SVIN_NBG0_CHPNDR_START+iFound*64),big_buffer+iPointer,64);
                        p32 = (int*)_SVIN_NBG0_PNDR_START;
                        break;
                    case 1:
                        memcpy((char*)(_SVIN_NBG1_CHPNDR_START+iFound*64),big_buffer+iPointer,64);
                        p32 = (int*)_SVIN_NBG1_PNDR_START;
                        break;
                    case 2:
                        memcpy((char*)(_SVIN_NBG2_CHPNDR_START+iFound*64),big_buffer+iPointer,64);
                        p32 = (int*)_SVIN_NBG2_PNDR_START;
                        break; 
                }
                //setting tile index
                switch (iLayer)
                {
                    case 0:
                        iOffset = (_SVIN_NBG0_CHPNDR_START - VDP2_VRAM_ADDR(0,0))/32;
                        if (x<64)
                            p32[y*64+x] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
                        else
                            p32[(y+64)*64+x-64] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
                        break;
                    case 1:
                        //not killing textbox, lines 44 thru 53
                        //if ((y<44) || (y>53))
                        {
                            iOffset = (_SVIN_NBG1_CHPNDR_START - VDP2_VRAM_ADDR(0,0))/32;
                            if (x<64)
                                p32[y*64+x] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
                            else
                                p32[(y+64)*64+x-64] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
                        }
                        break;
                    case 2:
                        //not killing textbox, lines 44 thru 53
                        if ((y<44) || (y>53))
                        {
                            iOffset = (_SVIN_NBG2_CHPNDR_START - VDP2_VRAM_ADDR(0,0))/32;
                            if (x<64)
                                p32[y*64+x] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
                            else
                                p32[(y+64)*64+x-64] = 0x00000000 | 0x100000*(iPaletteIndex) | (iOffset + iFound*2); //palette 0, transparency on
                        }
                        break; 
                }
                //moving pointer
                iPointer+=64;
            }
        }
    }

    //load palette

    switch (iPalette)
    {
        case 0:
            _svin_set_palette(iPaletteIndex,big_buffer+iPointer);
            break;
        case 1:
            _svin_set_palette_half_hi(iPaletteIndex,big_buffer+iPointer);
            break;
        case 2:
            _svin_set_palette_half_lo(iPaletteIndex,big_buffer+iPointer);
            break;
    }

    _svin_set_cycle_patterns_nbg();

    free(big_buffer);
}