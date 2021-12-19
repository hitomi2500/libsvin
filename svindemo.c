#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

#define MENU_ENTRY_COUNT 16


extern vdp1_cmdt_list_t *_svin_cmdt_list;

int
main(void)
{
        MEMORY_WRITE(32, SCU(ASR0), 0x23301FF0);

        _svin_filelist_fill(); //move this to init probably
        
        _svin_init();

        _svin_textbox_disable(); //filling textbox tiles with invisible data

        /*uint8_t * tmp;
        tmp = (uint8_t *)0x22000000;
        char str[1024];
        sprintf(str,"200=%x 203=%x %x %x 210=%x %x %x 211=%x %x %x ",
                tmp[0x0],
                tmp[0x30000],
                tmp[0x30001],
                tmp[0x30002],
                tmp[0x100000],
                tmp[0x100001],
                tmp[0x100002],
                tmp[0x110000],
                tmp[0x110001],
                tmp[0x110002]);
        _svin_textbox_print("minidump",str,"Lato_Black12",3,3);
        while(1);*/
       
        //load logo
        _svin_clear_palette(0);
        _svin_background_set("images/bg/yaul_logo.bg");
        //while(1);
        _svin_delay(1000);
        _svin_background_fade_to_black();

        _svin_run_script("SCRIPT_ENG.TXT");

        while(1);

}
