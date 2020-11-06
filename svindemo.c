#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "scroll_menu.h"
#include "svin.h"

#define MENU_ENTRY_COUNT 16

static void _frt_ovi_handler(void);
static void _vblank_out_handler(void *);

static void _menu_input(scroll_menu_state_t *);
static void _menu_update(scroll_menu_state_t *);
static void _menu_action(void *, menu_entry_t *);

static menu_entry_t _menu_entries[MENU_ENTRY_COUNT + 1];

static smpc_peripheral_digital_t _digital;

static iso9660_filelist_t _filelist;
static iso9660_filelist_entry_t _filelist_entries[ISO9660_FILELIST_ENTRIES_COUNT];

static uint16_t _frt_overflow_count = 0;

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

        _svin_clear_background();

        while(1)
        {
            for (unsigned int i =0; i< _filelist.entries_count; i++)
            {
                file_entry = &_filelist.entries[i];
                if (file_entry->size == 315392)
                {
                    _svin_clear_background();
                    file_entry2 = &_filelist.entries[i+1];
                    _svin_set_background(file_entry->starting_fad,file_entry2->starting_fad);
                    //delay
                    volatile int dummy=0;
                    for (dummy = 0; dummy < 10050000; dummy++)
                        ;
                }
            }
        }


        while(1);

        scroll_menu_state_t menu_state;

        scroll_menu_init(&menu_state);
        scroll_menu_input_set(&menu_state, _menu_input);
        scroll_menu_update_set(&menu_state, _menu_update);
        scroll_menu_entries_set(&menu_state, _menu_entries);

        menu_state.view_height = MENU_ENTRY_COUNT - 1;
        menu_state.top_index = 0;
        menu_state.bottom_index = _filelist.entries_count;

        menu_state.flags = SCROLL_MENU_STATE_ENABLED | SCROLL_MENU_STATE_INPUT_ENABLED;

        while (true) {
                smpc_peripheral_process();
                smpc_peripheral_digital_port(1, &_digital);

                dbgio_printf("[H[2J");

                scroll_menu_update(&menu_state);

                dbgio_flush();
                vdp_sync();
        }
}


void
user_init(void)
{
        vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_NONE, VDP2_TVMD_HORZ_NORMAL_A,
            VDP2_TVMD_VERT_224);

        vdp2_scrn_back_screen_color_set(VDP2_VRAM_ADDR(3, 0x01FFFE),
            COLOR_RGB1555(1, 0, 3, 15));

        vdp_sync_vblank_out_set(_vblank_out_handler);

        cpu_frt_init(CPU_FRT_CLOCK_DIV_128);

        cpu_intc_mask_set(0);

        dbgio_dev_default_init(DBGIO_DEV_VDP2_ASYNC);
        dbgio_dev_font_load();
        dbgio_dev_font_load_wait();

        vdp2_tvmd_display_set();
}

static void
_vblank_out_handler(void *work __unused)
{
        smpc_peripheral_intback_issue();
}

static void
_frt_ovi_handler(void)
{
        _frt_overflow_count++;
}

static void
_menu_input(scroll_menu_state_t *menu_state)
{
        if ((_digital.held.button.down) != 0) {
                scroll_menu_cursor_down(menu_state);
        } else if ((_digital.held.button.up) != 0) {
                scroll_menu_cursor_up(menu_state);
        } else if ((_digital.held.button.a) != 0) {
                scroll_menu_action_call(menu_state);
        }
}

static void
_menu_update(scroll_menu_state_t *menu_state)
{
        for (int8_t i = 0; i <= menu_state->view_height; i++) {
                menu_entry_t *menu_entry;
                menu_entry = &_menu_entries[i];

                uint32_t y;
                y = scroll_menu_local_cursor(menu_state) + i;

                menu_entry->text = _filelist.entries[y].name;
                menu_entry->action = _menu_action;
        }

        _menu_entries[MENU_ENTRY_COUNT].text = NULL;
        _menu_entries[MENU_ENTRY_COUNT].action = NULL;
}

static void
_menu_action(void *state_ptr, menu_entry_t *menu_entry __unused)
{
        scroll_menu_state_t *menu_state;
        menu_state = state_ptr;

        uint32_t i = scroll_menu_cursor(menu_state);

        iso9660_filelist_entry_t *file_entry;
        file_entry = &_filelist.entries[i];

        dbgio_printf("\n\nLoading %s, FAD: %li, %i sectors...\n",
            file_entry->name,
            file_entry->starting_fad,
            file_entry->sector_count);

        dbgio_flush();
        vdp_sync();

        cpu_frt_ovi_set(_frt_ovi_handler);

        cpu_frt_count_set(0);

        /* Reset overflow counter after setting the FRT count to zero in case
         * there's an FRT overflow interrupt */
        _frt_overflow_count = 0;

        /* Loop through and copy each sector, one at a time */
        for (uint32_t sector = 0; sector < file_entry->sector_count; sector++) {
                int ret;
                ret = cd_block_sector_read(file_entry->starting_fad + sector, (void *)LWRAM(0));
                assert(ret == 0);
        }

        uint32_t ticks_count;
        ticks_count = (65536 * _frt_overflow_count) + cpu_frt_count_get();


        /* Use Q28.4 to calculate time in milliseconds */
        uint32_t time;
        time = ((ticks_count << 8) / ((1000 * CPU_FRT_NTSC_320_128_COUNT_1MS) << 4)) >> 4;

        dbgio_printf("\n\nLoaded! Took %lu ticks (~%lus).\n\nCheck LWRAM.\n\nWaiting 5 seconds\n", ticks_count, time);
        dbgio_flush();

        for (uint32_t i = 0; i < (5 * 60); i++) {
                vdp_sync();
        }
}
