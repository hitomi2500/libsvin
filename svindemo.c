#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "scroll_menu.h"
#include "svin.h"

#define MENU_ENTRY_COUNT 16

static iso9660_filelist_t _filelist;
static iso9660_filelist_entry_t _filelist_entries[ISO9660_FILELIST_ENTRIES_COUNT];

extern int cd_block_multiple_sectors_read(uint32_t fad, uint32_t number, uint8_t *output_buffer);


//uint8_t bg_romdisk[];

int
main(void)
{
        //iso9660_filelist_entry_t *file_entry;
        //iso9660_filelist_entry_t *file_entry2;
        _filelist.entries = _filelist_entries;
        _filelist.entries_count = 0;
        _filelist.entries_pooled_count = 0;

        /* Load the maximum number */
        iso9660_filelist_read(&_filelist, -1);

        _svin_init(&_filelist);

       _svin_background_set("int_dining_hall_people_day");

        while(1)
        {
           for (unsigned int i=0; i< 100; i++)
            {
                //_svin_background_set_by_index(i);
                _svin_background_update_by_index(i);
                //_svin_background_set("bus_stop");
                _svin_delay(1000);
            }
        }


        while(1);
}
