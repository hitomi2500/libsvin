#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "scroll_menu.h"
#include "svin.h"

#define MENU_ENTRY_COUNT 16

static iso9660_filelist_t _filelist;
static iso9660_filelist_entry_t _filelist_entries[ISO9660_FILELIST_ENTRIES_COUNT];

int
main(void)
{
        iso9660_filelist_entry_t *file_entry;
        iso9660_filelist_entry_t *file_entry2;
        _filelist.entries = _filelist_entries;
        _filelist.entries_count = 0;
        _filelist.entries_pooled_count = 0;

        /* Load the maximum number */
        iso9660_filelist_read(&_filelist, -1);

        _svin_init();

        //iso9660_dirent_t

        while(1)
        {
            for (unsigned int i =0; i< _filelist.entries_count; i++)
            {
                file_entry = &_filelist.entries[i];
                if (file_entry->size == 315392)
                {
                    //_svin_background_fade_to_black();
                    file_entry2 = &_filelist.entries[i+1];
                    _svin_background_update(file_entry->starting_fad,file_entry2->starting_fad);
                    _svin_delay(1000);
                }
            }
        }


        while(1);
}
