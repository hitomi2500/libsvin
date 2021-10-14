#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

#define MENU_ENTRY_COUNT 16


extern vdp1_cmdt_list_t *_svin_cmdt_list;

int
main(void)
{



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
#endif


        _svin_filelist_fill(); //move this to init probably

        _svin_init();

        MEMORY_WRITE(32, SCU(ASR0), 0x23301FF0);

        //load logo
        _svin_clear_palette(0);
        _svin_background_set("images/bg/yaul_logo.bg");
        _svin_delay(1000);
        _svin_background_fade_to_black();

        _svin_run_script("SCRIPT.TXT");

        while(1);

}
