#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

#define MENU_ENTRY_COUNT 16

extern uint32_t JumpLinks[512];


extern vdp1_cmdt_list_t *_svin_cmdt_list;

int
main(void)
{
        MEMORY_WRITE(32, SCU(ASR0), 0x23301FF0);

        
        _svin_init();

        _svin_textbox_disable(); //filling textbox tiles with invisible data

        _svin_background_set_no_filelist("BOOTLOGO.BG");
        _svin_delay(1000);
        _svin_background_fade_to_black();

        _svin_background_set_no_filelist("DISCLMR.BG");

        _svin_filelist_fill(); //move this to init probably

        _svin_menu_init("SCRIPT_ENG.MNU"); //this requires filelist to be loaded first

        //_svin_background_set("images/bg/yaul_logo.bg");
        //_svin_delay(1000);
        _svin_background_fade_to_black();

        _svin_menu_populate(0,"English");
        _svin_menu_populate(1,"Русский");
        if (0 == _svin_menu_activate())
        {
                //english chosen
                _svin_script_run("SCRIPT_ENG.TXT");
        }
        else
        {
                //russian chosen
                int i;
                //reinit menu links to russian
                _svin_menu_init("SCRIPT_RUS.MNU");
                //reinit label links to russian
                fad_t _jumplinks_fad;
                assert(true == _svin_filelist_search("SCRIPT_RUS.LBL",&_jumplinks_fad,&i));
                _svin_cd_block_sector_read(_jumplinks_fad, (uint8_t*)JumpLinks);
                _svin_script_run("SCRIPT_RUS.TXT");
        }

        

        while(1);

}
