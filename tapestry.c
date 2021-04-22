#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "scroll_menu.h"
#include "svin.h"

#define MENU_ENTRY_COUNT 16

static iso9660_filelist_t _filelist;
static iso9660_filelist_entry_t _filelist_entries[ISO9660_FILELIST_ENTRIES_COUNT];

extern vdp1_cmdt_list_t *_svin_cmdt_list;

static smpc_peripheral_digital_t _digital;

static inline bool __always_inline
_digital_dirs_pressed(void)
{
        return (_digital.pressed.raw & PERIPHERAL_DIGITAL_DIRECTIONS) != 0x0000;
}

int
main(void)
{
        _filelist.entries = _filelist_entries;
        _filelist.entries_count = 0;
        _filelist.entries_pooled_count = 0;

        int autoscroll = 1;
        int scroll = 0;

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
        _svin_tapestry_init();
        _svin_tapestry_load_position(&_filelist,"TAPESTRY.PAK",0);

        //for (int i=0;i<448*98;i++)
          //      _svin_tapestry_move_down();


        /*for (int i=0;i<800;i++)
                _svin_tapestry_move_down();
                
        for (int i=0;i<900;i++)
                _svin_tapestry_move_up();*/

        while (1)
        {
                smpc_peripheral_process();
                smpc_peripheral_digital_port(1, &_digital);

                if ( _digital_dirs_pressed()) 
                {
                        autoscroll = 0;
                }
                else  if ((_digital.held.button.a) != 0) 
                {
                        autoscroll = 1;
                }

                if (autoscroll != 0)
                {
                        if (autoscroll > 0)
                        {
                                _svin_tapestry_move_down(); 
                                scroll++;
                                if (scroll > 448*98)
                                      autoscroll = -1;        
                        }
                        else
                        {
                                _svin_tapestry_move_up(); 
                                scroll--;
                                if (scroll == 0)
                                      autoscroll = 1;        
                        }
                }
                else
                {
                        if ((_digital.pressed.raw & PERIPHERAL_DIGITAL_UP) != 0) 
                        {
                                _svin_tapestry_move_up(); 
                                scroll--;
                        }    
                        else if ((_digital.pressed.raw & PERIPHERAL_DIGITAL_DOWN) != 0) 
                        {
                                _svin_tapestry_move_down(); 
                                scroll++;
                        }   
                }
        }

        
        //_svin_background_set_by_index_half(0,0,0);
        //_svin_background_set_by_index_half(0,1,1);

        //vdp1_cmdt_t *cmdts;
        //cmdts = &_svin_cmdt_list->cmdts[0]; 
        //vdp1_cmdt_t *cmdt_sprite;
        //uint16_t _tmp;
        //uint16_t * p;

        _svin_delay(1000);

        

        /*for (int frame=1;frame<100;frame++)
        {
                //for (int part=0;part<1;part++)
                {
                        //load first part of next image into slot 2
                        _svin_background_set_by_index_half(frame,0,2);
                        //move all texture starts
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
                                _svin_delay(10);
                        }

                        _svin_background_set_by_index_half(frame-1,1,0);
                        for (int k=0;k<4;k+=2)
                        {
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_A0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] - 44*112;
                                p[0] = _tmp;  
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_B0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] - 44*112;
                                p[0] = _tmp; 
                        }
                        _svin_background_set_by_index_half(frame,0,1);
                        for (int k=1;k<4;k+=2)
                        {
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_A0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] - 44*112;
                                p[0] = _tmp;  
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_B0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] - 44*112;
                                p[0] = _tmp; 
                        }
                        //while (1);
                        _svin_background_set_by_index_half(frame,1,2);
                        //while (1);

                         //move all texture starts again
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
                                _svin_delay(10);
                        }

                        _svin_background_set_by_index_half(frame,0,0);
                        for (int k=0;k<4;k+=2)
                        {
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_A0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] - 44*112;
                                p[0] = _tmp;  
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_B0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] - 44*112;
                                p[0] = _tmp; 
                        }
                        _svin_background_set_by_index_half(frame,1,1);
                        for (int k=1;k<4;k+=2)
                        {
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_A0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] - 44*112;
                                p[0] = _tmp;  
                                p = (uint16_t*)VDP1_VRAM(0x08 + _SVIN_VDP1_ORDER_SPRITE_B0_INDEX * 0x20 + k*0x20);
                                _tmp = p[0] - 44*112;
                                p[0] = _tmp; 
                        }
                        //while (1);
                }


        }*/

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
