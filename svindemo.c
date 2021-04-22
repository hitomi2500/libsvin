#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "scroll_menu.h"
#include "svin.h"

#define MENU_ENTRY_COUNT 16

static iso9660_filelist_t _filelist;
static iso9660_filelist_entry_t _filelist_entries[ISO9660_FILELIST_ENTRIES_COUNT];

extern vdp1_cmdt_list_t *_svin_cmdt_list;

int
main(void)
{
        _filelist.entries = _filelist_entries;
        _filelist.entries_count = 0;
        _filelist.entries_pooled_count = 0;

        /* Load the maximum number */
#ifdef _SVIN_DIRTY_STATIC_LINKING
        //for dirty linking, fill the filelist manually
        _filelist.entries[0].name[0] = 'B';
        _filelist.entries[0].name[1] = 'G';
        _filelist.entries[0].name[2] = '.';
        _filelist.entries[0].name[3] = 'P';
        _filelist.entries[0].name[4] = 'A';
        _filelist.entries[0].name[5] = 'K';
        _filelist.entries[0].name[6] = '\0';
        _filelist.entries[0].starting_fad = 128; //BG.PAK starts at 1M in CS0
        _filelist.entries[0].size = 636928; // DIRTY UPDATE ME!
        _filelist.entries[0].sector_count = (_filelist.entries[0].size - 1)/2048 + 1;
        _filelist.entries[1].name[0] = 'S';
        _filelist.entries[1].name[1] = 'L';
        _filelist.entries[1].name[2] = '.';
        _filelist.entries[1].name[3] = 'P';
        _filelist.entries[1].name[4] = 'A';
        _filelist.entries[1].name[5] = 'K';
        _filelist.entries[1].name[6] = '\0';
        _filelist.entries[1].starting_fad = 480; //SL.PAK starts at 2M in CS0
        _filelist.entries[1].size = 47104; // DIRTY UPDATE ME!
        _filelist.entries[1].sector_count = (_filelist.entries[1].size - 1)/2048 + 1;
        _filelist.entries_count = 2;
        _filelist.entries_pooled_count = 0;
#else
        iso9660_filelist_root_read(&_filelist, -1);
#endif



        

        _svin_init();

        //_svin_hang_test(9);   
        //while(1);   

        MEMORY_WRITE(32, SCU(ASR0), 0x23301FF0);

        _svin_background_load_index(&_filelist);
        //_svin_actor_debug_load_index(&_filelist);

        //load logo
        _svin_background_clear_palette(0);
        _svin_background_set("yaul_logo");
        _svin_delay(2000);
        _svin_background_fade_to_black();

        //_svin_background_set("int_dining_hall_people_day");
        //_svin_background_set_by_index(0);
        //_svin_actor_debug_load_test(&_filelist,"SL.PAK",0);
        //_svin_actor_debug_load_test(&_filelist,"US.PAK",1);

        _svin_background_set_by_index(0);
        //_svin_background_set_by_index_half(0,0,0);
        //_svin_background_set_by_index_half(0,1,1);

        //vdp1_cmdt_t *cmdts;
        //cmdts = &_svin_cmdt_list->cmdts[0]; 
        //vdp1_cmdt_t *cmdt_sprite;
        uint16_t _tmp;
        uint16_t * p;

        _svin_delay(1000);
        for (int frame=0;frame<1;frame+=2)
        {
                //load first part of next image into slot 2
                _svin_background_set_by_index_half(frame,0,2);
                //move all starts from slot 0 to slot 1
                for (int shift=0;shift<112; shift++)
                {
                        for (int k=0;k<4;k++)
                        {
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_A0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] + 44;//slot 0 to slot 1
                                p[0] = _tmp;
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_B0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] + 44;//slot 1 to slot 2
                                p[0] = _tmp;
                        }
                        _svin_delay(100);
                }

                //load second part of next image into slot 0
                _svin_background_set_by_index_half(frame,1,0);
                //move all starts from slot 1 to slot 2
                for (int k=0;k<4;k++)
                {
                        p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_B0_INDEX * 0x20 + k*0x20);
                        _tmp = p[0] - 44*112*3;//slot 2 to slot 0
                        p[0] = _tmp;
                }
                for (int shift=0;shift<112; shift++)
                        {
                        for (int k=0;k<4;k++)
                        {
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_A0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] + 44;//slot 1 to slot 2
                                p[0] = _tmp;
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_B0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] + 44;//slot 0 to slot 1
                                p[0] = _tmp;
                        }
                        _svin_delay(100);
                }

        }

        while(1);

        while(1)
        {
           for (unsigned int i=0; i< 100; i++)
            {
                //_svin_background_set _by_index(i);
                _svin_background_update_by_index(i);
                //_svin_background_set("bus_stop");
                _svin_delay(1000);
            }
        }


        while(1);
}
