#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

#define MENU_ENTRY_COUNT 16

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
        int autoscroll = 1;
        int scroll = 0;

        MEMORY_WRITE(32, SCU(ASR0), 0x23301FF0);

        _svin_init();
		_svin_textbox_disable();
				
		bool bCD_Ok = _svin_filelist_fill(); //move this to init probably
		assert (true == bCD_Ok);

        //load logo
        _svin_clear_palette(0);
        _svin_background_set("BOOTLOGO.BG");
        _svin_delay(2000);
        _svin_background_fade_to_black();

        _svin_tapestry_init();
        _svin_tapestry_load_position("TAPESTRY.PAK",0);
		
		while (1);

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

}
