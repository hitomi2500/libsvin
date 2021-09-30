#include <yaul.h>
#include <vdp1/vram.h>
#include <tga.h>
#include <svin.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <cd-block_multiread.h>

extern int cd_block_multiple_sectors_read(uint32_t fad, uint32_t number, uint8_t *output_buffer);
static void _svin_vblank_out_handler(void *);

#ifdef _SVIN_DIRTY_svin_background_set_by_index_half_STATIC_LINKING
    #define _svin_cd_block_multiple_sectors_read _svin_dirty_multiple_sectors_read
    #define _svin_cd_block_sector_read _svin_dirty_sector_read
extern int _svin_dirty_sector_read(uint32_t fad, uint8_t *output_buffer)
{
    memcpy(output_buffer,(uint8_t*)CS0(fad*2048),2048);
    return 0;
}
extern int _svin_dirty_multiple_sectors_read(uint32_t fad, uint32_t number, uint8_t *output_buffer)
{
    memcpy(output_buffer,(uint8_t*)CS0(fad*2048),2048*number);
    return 0;
}
#else
    #define _svin_cd_block_multiple_sectors_read cd_block_multiple_sectors_read
    #define _svin_cd_block_sector_read cd_block_sector_read
    #define _svin_cd_block_sector_read_request cd_block_sector_read_request
    #define _svin_cd_block_sector_read_process cd_block_sector_read_process
    #define _svin_cd_block_sector_read_flush cd_block_sector_read_flush
#endif

static smpc_peripheral_digital_t _digital;

fad_t _svin_background_pack_fad;
uint8_t *_svin_background_index;
uint16_t _svin_background_files_number;
//fad_t _svin_actor_pack_fad[64];
//uint8_t *_svin_actor_index[64];
fad_t _svin_actor_debug_pack_fad;
uint8_t *_svin_actor_debug_index;
uint16_t _svin_actor_debug_files_number;
//uint8_t *_svin_actor_index;
uint16_t _svin_actor_left[4];
uint16_t _svin_actor_top[4];
uint16_t _svin_actor_sizex[4];
uint16_t _svin_actor_sizey[4];
vdp1_cmdt_list_t *_svin_cmdt_list;
vdp1_cmdt_list_t *_svin_tapestry_cmdt_list_1;
vdp1_cmdt_list_t *_svin_tapestry_cmdt_list_2;
uint8_t _svin_init_done;
fad_t _svin_tapestry_pack_fad;
int _svin_tapestry_framebuffer_start;
int _svin_tapestry_data_start;
int _svin_tapestry_movement_active;

/*void _svin_hang_test(int i)
{
                vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_NONE, VDP2_TVMD_HORZ_NORMAL_A,
            VDP2_TVMD_VERT_224);

        vdp2_scrn_back_screen_color_set(VDP2_VRAM_ADDR(3, 0x01FFFE),
            COLOR_RGB1555(1, 0, 3, 15));

        cpu_intc_mask_set(0);

        vdp2_tvmd_display_set();
        
        dbgio_dev_default_init(DBGIO_DEV_VDP2_ASYNC);
        dbgio_dev_font_load();
        dbgio_dev_font_load_wait();

        dbgio_printf("THIIISSS IIIISSS SVIIIIIN %i\n",i);   
        dbgio_flush();
        vdp_sync();
}*/

void _svin_delay(int milliseconds)
{
    //delay
    volatile int dummy = 0;
    for (dummy = 0; dummy < 3000 * milliseconds; dummy++)
        ;
}

void _svin_set_cycle_patterns_cpu()
{
    // switching everything to CPU access.
    // no data could be displayed during this, so be sure to blank/disable the screen beforehand

    struct vdp2_vram_cycp vram_cycp;

    vram_cycp.pt[0].t0 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[0].t1 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[0].t2 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[0].t3 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[0].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[0].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[0].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[0].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

    vram_cycp.pt[1].t0 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[1].t1 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[1].t2 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[1].t3 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[1].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[1].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[1].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[1].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

    vram_cycp.pt[2].t0 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[2].t1 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[2].t2 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[2].t3 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[2].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[2].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[2].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[2].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

    vram_cycp.pt[3].t0 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[3].t1 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[3].t2 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[3].t3 = VDP2_VRAM_CYCP_CPU_RW;
    vram_cycp.pt[3].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[3].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[3].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[3].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

    vdp2_vram_cycp_set(&vram_cycp);
    vdp_sync();
}

void _svin_set_cycle_patterns_nbg()
{
    //swithcing everything to NBG accesses, CPU can't write data anymore

    struct vdp2_vram_cycp vram_cycp;

    vram_cycp.pt[0].t0 = VDP2_VRAM_CYCP_PNDR_NBG0;
    vram_cycp.pt[0].t1 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
    vram_cycp.pt[0].t2 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
    vram_cycp.pt[0].t3 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
    vram_cycp.pt[0].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[0].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[0].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[0].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

    vram_cycp.pt[1].t0 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
    vram_cycp.pt[1].t1 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
    vram_cycp.pt[1].t2 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
    vram_cycp.pt[1].t3 = VDP2_VRAM_CYCP_PNDR_NBG2;
    vram_cycp.pt[1].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[1].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[1].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[1].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

    vram_cycp.pt[2].t0 = VDP2_VRAM_CYCP_PNDR_NBG1;
    vram_cycp.pt[2].t1 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
    vram_cycp.pt[2].t2 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
    vram_cycp.pt[2].t3 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
    vram_cycp.pt[2].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[2].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[2].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[2].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

    vram_cycp.pt[3].t0 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
    vram_cycp.pt[3].t1 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
    vram_cycp.pt[3].t2 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
    vram_cycp.pt[3].t3 = VDP2_VRAM_CYCP_CHPNDR_NBG2;
    vram_cycp.pt[3].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[3].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[3].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
    vram_cycp.pt[3].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

    vdp2_vram_cycp_set(&vram_cycp);
    vdp_sync();

    //enable transparency for NBG1 over NBG0 (might not work with sprites between
    MEMORY_WRITE(16, VDP2(CCCTL), 0x0000); //disable cc both layers
    //MEMORY_WRITE(16, VDP2(CCRNA), 0x0C00); //enable cc for NBG1
}

void _svin_init()
{
    int *_pointer32;
    _svin_init_done = 0;

    //-------------- setup VDP2 -------------------

    //clearing vdp2
    vdp2_tvmd_display_clear();

    //setup nbg0
    struct vdp2_scrn_cell_format format;
    memset(&format, 0x00, sizeof(format));

    format.scroll_screen = VDP2_SCRN_NBG0;
    format.cc_count = VDP2_SCRN_CCC_PALETTE_256;
    format.character_size = (1 * 1);
    format.pnd_size = 2;
    format.auxiliary_mode = 1;
    format.cp_table = 0;
    format.color_palette = 0;
    format.plane_size = (2 * 1);
    format.sf_type = 0;//VDP2_SCRN_SF_TYPE_COLOR_CALCULATION;
    format.sf_code = VDP2_SCRN_SF_CODE_A;
    format.sf_mode = 0;
    format.map_bases.plane_a = _SVIN_NBG0_PNDR_START;

    vdp2_scrn_cell_format_set(&format);
    vdp2_scrn_priority_set(VDP2_SCRN_NBG0, 3);
    vdp2_scrn_display_set(VDP2_SCRN_NBG0, true);
    vdp2_cram_mode_set(1);

    //_svin_hang_test(2); 
    //while(1);

    //setup nbg1
    format.scroll_screen = VDP2_SCRN_NBG1;
    format.cc_count = VDP2_SCRN_CCC_PALETTE_256;
    format.character_size = (1 * 1);
    format.pnd_size = 2;
    format.auxiliary_mode = 1;
    format.cp_table = 0;
    format.color_palette = 0;
    format.plane_size = (2 * 1);
    format.sf_type = 0;//VDP2_SCRN_SF_TYPE_COLOR_CALCULATION;
    format.sf_code = VDP2_SCRN_SF_CODE_A;
    format.sf_mode = 0;
    format.map_bases.plane_a = _SVIN_NBG1_PNDR_START;

    vdp2_scrn_cell_format_set(&format);
    vdp2_scrn_priority_set(VDP2_SCRN_NBG1, 5);
    vdp2_scrn_display_set(VDP2_SCRN_NBG1, true);

    //setup nbg2
    format.scroll_screen = VDP2_SCRN_NBG2;
    format.cc_count = VDP2_SCRN_CCC_PALETTE_256;
    format.character_size = (1 * 1);
    format.pnd_size = 2;
    format.auxiliary_mode = 1;
    format.cp_table = 0;
    format.color_palette = 0;
    format.plane_size = (2 * 1);
    format.sf_type = 0;//VDP2_SCRN_SF_TYPE_COLOR_CALCULATION;
    format.sf_code = VDP2_SCRN_SF_CODE_A;
    format.sf_mode = 0;
    format.map_bases.plane_a = _SVIN_NBG2_PNDR_START;

    vdp2_scrn_cell_format_set(&format);
    vdp2_scrn_priority_set(VDP2_SCRN_NBG2, 6);
    vdp2_scrn_display_set(VDP2_SCRN_NBG2, true);

    vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_DOUBLE, VDP2_TVMD_HORZ_HIRESO_B, VDP2_TVMD_VERT_224); //704x448 works

    color_rgb1555_t bs_color;
    bs_color = COLOR_RGB1555(1, 0, 0, 0);
    vdp2_scrn_back_screen_color_set(VDP2_VRAM_ADDR(3, 0x01FFFE), bs_color);

    vdp2_sprite_priority_set(0, 2);
    vdp2_sprite_priority_set(1, 2);
    vdp2_sprite_priority_set(2, 2);
    vdp2_sprite_priority_set(3, 2);
    vdp2_sprite_priority_set(4, 2);
    vdp2_sprite_priority_set(5, 2);
    vdp2_sprite_priority_set(6, 2);
    vdp2_sprite_priority_set(7, 2);

    //setting cycle patterns for cpu access
    _svin_set_cycle_patterns_cpu();

    vdp_sync_vblank_out_set(_svin_vblank_out_handler);
    
    vdp2_tvmd_display_set();
    //cpu_intc_mask_set(0); //?????
    vdp_sync();

    //-------------- setup VDP1 -------------------
    _svin_cmdt_list = vdp1_cmdt_list_alloc(_SVIN_VDP1_ORDER_COUNT);

    static const int16_vec2_t local_coord_ul =
        INT16_VEC2_INITIALIZER(0,
                               0);

    static const vdp1_cmdt_draw_mode_t sprite_draw_mode = {
        .raw = 0x0000,
        .bits.pre_clipping_disable = true};

    assert(_svin_cmdt_list != NULL);

    vdp1_cmdt_t *cmdts;
    cmdts = &_svin_cmdt_list->cmdts[0];

    (void)memset(&cmdts[0], 0x00, sizeof(vdp1_cmdt_t) * _SVIN_VDP1_ORDER_COUNT);

    _svin_cmdt_list->count = _SVIN_VDP1_ORDER_COUNT;

    vdp1_cmdt_normal_sprite_set(&cmdts[_SVIN_VDP1_ORDER_SPRITE_A0_INDEX]);
    vdp1_cmdt_param_draw_mode_set(&cmdts[_SVIN_VDP1_ORDER_SPRITE_A0_INDEX], sprite_draw_mode);
    vdp1_cmdt_normal_sprite_set(&cmdts[_SVIN_VDP1_ORDER_SPRITE_A1_INDEX]);
    vdp1_cmdt_param_draw_mode_set(&cmdts[_SVIN_VDP1_ORDER_SPRITE_A1_INDEX], sprite_draw_mode);
    vdp1_cmdt_normal_sprite_set(&cmdts[_SVIN_VDP1_ORDER_SPRITE_B0_INDEX]);
    vdp1_cmdt_param_draw_mode_set(&cmdts[_SVIN_VDP1_ORDER_SPRITE_B0_INDEX], sprite_draw_mode);
    vdp1_cmdt_normal_sprite_set(&cmdts[_SVIN_VDP1_ORDER_SPRITE_B1_INDEX]);
    vdp1_cmdt_param_draw_mode_set(&cmdts[_SVIN_VDP1_ORDER_SPRITE_B1_INDEX], sprite_draw_mode);

    vdp1_cmdt_system_clip_coord_set(&cmdts[_SVIN_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX]);
    vdp1_cmdt_jump_assign(&cmdts[_SVIN_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX], _SVIN_VDP1_ORDER_LOCAL_COORDS_A_INDEX * 4);

    vdp1_cmdt_local_coord_set(&cmdts[_SVIN_VDP1_ORDER_LOCAL_COORDS_A_INDEX]);
    vdp1_cmdt_local_coord_set(&cmdts[_SVIN_VDP1_ORDER_LOCAL_COORDS_B_INDEX]);
    vdp1_cmdt_param_vertex_set(&cmdts[_SVIN_VDP1_ORDER_LOCAL_COORDS_A_INDEX],
                               CMDT_VTX_LOCAL_COORD, &local_coord_ul);
    vdp1_cmdt_param_vertex_set(&cmdts[_SVIN_VDP1_ORDER_LOCAL_COORDS_B_INDEX],
                               CMDT_VTX_LOCAL_COORD, &local_coord_ul);

    vdp1_cmdt_end_set(&cmdts[_SVIN_VDP1_ORDER_DRAW_END_A_INDEX]);
    vdp1_cmdt_end_set(&cmdts[_SVIN_VDP1_ORDER_DRAW_END_B_INDEX]);
#define VDP1_FBCR_DIE (0x0008)
    MEMORY_WRITE(16, VDP1(FBCR), VDP1_FBCR_DIE);
    
    vdp1_vram_partitions_set(64,//VDP1_VRAM_DEFAULT_CMDT_COUNT,
                              0x7F000, //  VDP1_VRAM_DEFAULT_TEXTURE_SIZE,
                               0,//  VDP1_VRAM_DEFAULT_GOURAUD_COUNT,
                               0);//  VDP1_VRAM_DEFAULT_CLUT_COUNT);

            static vdp1_env_t vdp1_env = {
                .erase_color = COLOR_RGB1555(1, 0, 0, 0),
                .erase_points[0] = {
                        .x = 0,
                        .y = 0
                },
                .erase_points[1] = {
                        .x = _SVIN_SCREEN_WIDTH - 1,
                        .y = _SVIN_SCREEN_HEIGHT -1
                },
                .bpp = VDP1_ENV_BPP_8,
                .rotation = VDP1_ENV_ROTATION_0,
                .color_mode = VDP1_ENV_COLOR_MODE_PALETTE,
                .sprite_type = 0xC
        };

        vdp1_env_set(&vdp1_env);

    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    vdp1_cmdt_color_bank_t dummy_bank;
    dummy_bank.raw = 0;

    vdp1_cmdt_t *cmdt_sprite;
    cmdt_sprite = &cmdts[_SVIN_VDP1_ORDER_SPRITE_A0_INDEX];
    cmdt_sprite->cmd_xa = 0;
    cmdt_sprite->cmd_ya = 0;
    cmdt_sprite->cmd_size = 0x2CE0;
    cmdt_sprite->cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0) ) / 8;
    vdp1_cmdt_param_color_mode4_set(cmdt_sprite, dummy_bank);
    vdp1_cmdt_param_color_bank_set(cmdt_sprite, dummy_bank);
    cmdt_sprite->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now
    cmdt_sprite = &cmdts[_SVIN_VDP1_ORDER_SPRITE_A1_INDEX];
    cmdt_sprite->cmd_xa = 352;
    cmdt_sprite->cmd_ya = 0;
    cmdt_sprite->cmd_size = 0x2CE0;
    cmdt_sprite->cmd_srca = ((int)vdp1_vram_partitions.texture_base - VDP1_VRAM(0) + _SVIN_SCREEN_WIDTH*_SVIN_SCREEN_HEIGHT/4 ) / 8;
    vdp1_cmdt_param_color_mode4_set(cmdt_sprite, dummy_bank);
    vdp1_cmdt_param_color_bank_set(cmdt_sprite, dummy_bank);
    vdp1_cmdt_jump_assign(cmdt_sprite, _SVIN_VDP1_ORDER_DRAW_END_A_INDEX * 4);//skipping A2 and A3
    cmdt_sprite->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now
    cmdt_sprite = &cmdts[_SVIN_VDP1_ORDER_SPRITE_B0_INDEX];
    cmdt_sprite->cmd_xa = 0;
    cmdt_sprite->cmd_ya = 0;
    cmdt_sprite->cmd_size = 0x2CE0;
    cmdt_sprite->cmd_srca = ((int)vdp1_vram_partitions.texture_base - VDP1_VRAM(0) + 2*_SVIN_SCREEN_WIDTH*_SVIN_SCREEN_HEIGHT/4 ) / 8;
    vdp1_cmdt_param_color_mode4_set(cmdt_sprite, dummy_bank);
    vdp1_cmdt_param_color_bank_set(cmdt_sprite, dummy_bank);
    cmdt_sprite->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now
    cmdt_sprite = &cmdts[_SVIN_VDP1_ORDER_SPRITE_B1_INDEX];
    cmdt_sprite->cmd_xa = 352;
    cmdt_sprite->cmd_ya = 0;
    cmdt_sprite->cmd_size = 0x2CE0;
    cmdt_sprite->cmd_srca = ((int)vdp1_vram_partitions.texture_base - VDP1_VRAM(0) + 3*_SVIN_SCREEN_WIDTH*_SVIN_SCREEN_HEIGHT/4 ) / 8;
    vdp1_cmdt_param_color_mode4_set(cmdt_sprite, dummy_bank);
    vdp1_cmdt_param_color_bank_set(cmdt_sprite, dummy_bank);
    vdp1_cmdt_jump_assign(cmdt_sprite, _SVIN_VDP1_ORDER_DRAW_END_B_INDEX * 4);//skipping B2 and B3
    cmdt_sprite->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now

    vdp1_cmdt_t *cmdt_system_clip_coords;
    cmdt_system_clip_coords = &cmdts[_SVIN_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX];

    cmdt_system_clip_coords->cmd_xc = _SVIN_SCREEN_WIDTH - 1;
    cmdt_system_clip_coords->cmd_yc = _SVIN_SCREEN_HEIGHT - 1;

    vdp1_sync_cmdt_list_put(_svin_cmdt_list, 0, NULL, NULL);
    vdp_sync();

    //-------------- setup pattern names -------------------

    //writing pattern names for nbg0
    //starting with plane 0
    _pointer32 = (int *)_SVIN_NBG0_PNDR_START;
    for (unsigned int i = 0; i < _SVIN_NBG0_PNDR_SIZE / sizeof(int); i++)
    {
        _pointer32[i] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
    }

    //writing pattern names for nbg1
    //nbg1  is mostly transparent, so fill with that one
    _pointer32 = (int *)_SVIN_NBG1_PNDR_START;
    for (unsigned int i = 0; i < _SVIN_NBG1_PNDR_SIZE / sizeof(int); i++)
    {
        _pointer32[i] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
    }

    //writing pattern names for nbg2
    //nbg2  is mostly transparent, so fill with that one
    _pointer32 = (int *)_SVIN_NBG2_PNDR_START;
    for (unsigned int i = 0; i < _SVIN_NBG2_PNDR_SIZE / sizeof(int); i++)
    {
        _pointer32[i] = 0x10000000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
    }

    //-------------- setup character pattern names -------------------

    //clearing character pattern names data for nbg0
    _pointer32 = (int *)_SVIN_NBG0_CHPNDR_START;
    for (unsigned int i = 0; i < _SVIN_NBG0_CHPNDR_SIZE / sizeof(int); i++)
    {
        _pointer32[i] = 0;
    }

    //clearing character pattern names data for nbg1
    _pointer32 = (int *)_SVIN_NBG1_CHPNDR_START;
    for (unsigned int i = 0; i < _SVIN_NBG1_CHPNDR_SIZE / sizeof(int); i++)
    {
        _pointer32[i] = 0;
    }

    //clearing character pattern names data for nbg2
    _pointer32 = (int *)_SVIN_NBG2_CHPNDR_START;
    for (unsigned int i = 0; i < _SVIN_NBG2_CHPNDR_SIZE / sizeof(int); i++)
    {
        _pointer32[i] = 0;
    }

    //setting up "transparent" character for nbg1
    _pointer32 = (int *)_SVIN_NBG1_CHPNDR_SPECIALS_ADDR;
    for (unsigned int i = 0; i < _SVIN_CHARACTER_BYTES / sizeof(int); i++)
    {
        _pointer32[i] = 0;
    }

    //setting up "semi-transparent" character for nbg1
    _pointer32 = (int *)(_SVIN_NBG1_CHPNDR_SPECIALS_ADDR + _SVIN_CHARACTER_BYTES);
    for (unsigned int i = 0; i < _SVIN_CHARACTER_BYTES / sizeof(int); i++)
    {
        _pointer32[i] = 0x7F7F7F7F;
    }

    //-------------- setup palettes -------------------

    //setup default palettes
    uint8_t temp_pal[3 * 256];
    //grayscale gradient
    for (int i = 0; i < 256; i++)
    {
        temp_pal[i * 3] = i;     
        temp_pal[i * 3 + 1] = i; 
        temp_pal[i * 3 + 2] = i; 
    }
    _svin_set_palette(0, temp_pal);
    //red
    memset(temp_pal,0,sizeof(temp_pal));
    for (int i = 0; i < 256; i++)
    {
        temp_pal[i * 3] = i;      
    }
    _svin_set_palette(1, temp_pal);
    //green
    memset(temp_pal,0,sizeof(temp_pal));
    for (int i = 0; i < 256; i++)
    {
       temp_pal[i * 3 + 1] = i;      
    }
    _svin_set_palette(2, temp_pal);
    //blue
    memset(temp_pal,0,sizeof(temp_pal));
    for (int i = 0; i < 256; i++)
    {
       temp_pal[i * 3 + 2] = i;      
    }
    _svin_set_palette(3, temp_pal);
    //cyan
    memset(temp_pal,0,sizeof(temp_pal));
    for (int i = 0; i < 256; i++)
    {
        temp_pal[i * 3 + 1] = i; 
        temp_pal[i * 3 + 2] = i;    
    }
    _svin_set_palette(4, temp_pal);
    //magenta
    memset(temp_pal,0,sizeof(temp_pal));
    for (int i = 0; i < 256; i++)
    {
        temp_pal[i * 3] = i;     
        temp_pal[i * 3 + 2] = i;    
    }
    _svin_set_palette(5, temp_pal);
    //yellow
    memset(temp_pal,0,sizeof(temp_pal));
    for (int i = 0; i < 256; i++)
    {
        temp_pal[i * 3] = i;     
        temp_pal[i * 3 + 1] = i; 
    }
    _svin_set_palette(6, temp_pal);  

    _svin_textbox_init();
    _svin_sprite_init();

    //setting cycle patterns for nbg access
    _svin_set_cycle_patterns_nbg();

    //-------------- init end -------------------  
    //vdp1_cmdt_jump_assign(&_svin_cmdt_list->cmdts[_SVIN_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX], _SVIN_VDP1_ORDER_LOCAL_COORDS_B_INDEX * 4);
    _svin_init_done = 1;
}

//---------------------------------------------- Palette stuff ----------------------------------------------------

void _svin_set_palette(int number, uint8_t *pointer)
{
    uint16_t *my_vdp2_cram = (uint16_t *)VDP2_VRAM_ADDR(8, 0x200 * number);
    for (int i = 0; i < 256; i++)
    {
        my_vdp2_cram[i] = (((pointer[i * 3 + 2] & 0xF8) << 7) |
                           ((pointer[i * 3 + 1] & 0xF8) << 2) |
                           ((pointer[i * 3 + 0] & 0xF8) >> 3));
    }
}

void _svin_set_palette_half_lo(int number, uint8_t * pointer)
{
    uint16_t *my_vdp2_cram = (uint16_t *)VDP2_VRAM_ADDR(8, 0x200 * number);
    for (int i = 0; i < 128; i++)
    {
        my_vdp2_cram[i] = (((pointer[i * 3 + 2] & 0xF8) << 7) |
                           ((pointer[i * 3 + 1] & 0xF8) << 2) |
                           ((pointer[i * 3 + 0] & 0xF8) >> 3));
    }
}

void _svin_set_palette_half_hi(int number, uint8_t * pointer)
{
    uint16_t *my_vdp2_cram = (uint16_t *)VDP2_VRAM_ADDR(8, 0x200 * number);
    for (int i = 128; i < 256; i++)
    {
        my_vdp2_cram[i] = (((pointer[i * 3 + 2] & 0xF8) << 7) |
                           ((pointer[i * 3 + 1] & 0xF8) << 2) |
                           ((pointer[i * 3 + 0] & 0xF8) >> 3));
    }
}

void _svin_clear_palette(int number)
{
    uint16_t *my_vdp2_cram = (uint16_t *)VDP2_VRAM_ADDR(8, 0x200 * number);
    for (int i = 0; i < 256; i++)
    {
        my_vdp2_cram[i] = 0;
    }
}



//---------------------------------------------- Tapestry stuff ----------------------------------------------------

void 
_svin_tapestry_init()
{
    _svin_tapestry_movement_active = 0;

    //basically it's a partial VDP1 reconfiguration
    //instead of 4 352x224 BG tiles we work with 896 quads with 352x1 size
    //so the list is divided into two alternative 448-quad lists
    //to keep compatible with a "normal" 4-quad setup we keep the first 16 commands same, 
    //but insert jump addresses into _SVIN_VDP1_ORDER_LOCAL_COORDS_A_INDEX and
    // _SVIN_VDP1_ORDER_LOCAL_COORDS_B_INDEX commands
    //First one jumps to command 16, and the second to the command 480.
    //Commands space is reserved for 1024 commands, i.e. 32 kB

    //-------------- setup VDP1 -------------------

    //allocating a big command lists, so that we can upload them in a single vblank
    _svin_tapestry_cmdt_list_1 = vdp1_cmdt_list_alloc(449); 
    _svin_tapestry_cmdt_list_2 = vdp1_cmdt_list_alloc(449); 

    static const int16_vec2_t local_coord_ul =
        INT16_VEC2_INITIALIZER(0,0);
    
    vdp1_cmdt_t cmdt_buf;
    vdp1_cmdt_t * cmdt_p;

    vdp1_cmdt_color_bank_t dummy_bank;
    dummy_bank.raw = 0;

    vdp1_vram_partitions_set(1023,//VDP1_VRAM_DEFAULT_CMDT_COUNT,
                              0x78000, //  VDP1_VRAM_DEFAULT_TEXTURE_SIZE,
                               0,//  VDP1_VRAM_DEFAULT_GOURAUD_COUNT,
                               0);//  VDP1_VRAM_DEFAULT_CLUT_COUNT);

    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    //dirty vram clean
    for (int i=0;i<224;i++)
    {
        (void)memset((void*)VDP1_VRAM(32768+i*1408), 0, 1408);
    }  

    //frame A branching first
    (void)memset(&cmdt_buf, 0x00, sizeof(vdp1_cmdt_t));
    vdp1_cmdt_local_coord_set(&cmdt_buf);
    vdp1_cmdt_param_vertex_set(&cmdt_buf,CMDT_VTX_LOCAL_COORD, &local_coord_ul);
    vdp1_cmdt_jump_assign(&cmdt_buf,16*4);
    vdp1_sync_cmdt_put(&cmdt_buf,1,32*_SVIN_VDP1_ORDER_LOCAL_COORDS_A_INDEX,NULL,NULL);
    vdp_sync();

    //filling first 224 quads
    for (int i=0;i<224;i++)
    {
        cmdt_p = &_svin_tapestry_cmdt_list_1->cmdts[i];
        (void)memset(cmdt_p, 0x00, sizeof(vdp1_cmdt_t));
        cmdt_p->cmd_xa = 0;
        cmdt_p->cmd_ya = i;
        cmdt_p->cmd_size = 0x2C01;
        cmdt_p->cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*i+0) / 8;
        vdp1_cmdt_param_color_mode4_set(cmdt_p, dummy_bank);
        vdp1_cmdt_param_color_bank_set(cmdt_p, dummy_bank);
        cmdt_p->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now
    }
    //filling another 224 quads
    for (int i=0;i<224;i++)
    {
        cmdt_p = &_svin_tapestry_cmdt_list_1->cmdts[i+224];
        (void)memset(cmdt_p, 0x00, sizeof(vdp1_cmdt_t));
        cmdt_p->cmd_xa = 352;
        cmdt_p->cmd_ya = i;
        cmdt_p->cmd_size = 0x2C01;
        cmdt_p->cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*i+352) / 8;
        vdp1_cmdt_param_color_mode4_set(cmdt_p, dummy_bank);
        vdp1_cmdt_param_color_bank_set(cmdt_p, dummy_bank);
        cmdt_p->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now
    }

    cmdt_p = &_svin_tapestry_cmdt_list_1->cmdts[448];
    (void)memset(cmdt_p, 0x00, sizeof(vdp1_cmdt_t));
    vdp1_cmdt_end_set(cmdt_p);
 
    _svin_tapestry_cmdt_list_1->count = 449;

    vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_1, 32*16, NULL, NULL);
    vdp_sync();

    //now frame B
    (void)memset(&cmdt_buf, 0x00, sizeof(vdp1_cmdt_t));
    vdp1_cmdt_local_coord_set(&cmdt_buf);
    vdp1_cmdt_param_vertex_set(&cmdt_buf,CMDT_VTX_LOCAL_COORD, &local_coord_ul);
    vdp1_cmdt_jump_assign(&cmdt_buf,480*4);
    vdp1_sync_cmdt_put(&cmdt_buf,1,32*_SVIN_VDP1_ORDER_LOCAL_COORDS_B_INDEX,NULL,NULL);
    vdp_sync();

    //filling first 224 quads
    for (int i=0;i<224;i++)
    {
        cmdt_p = &_svin_tapestry_cmdt_list_2->cmdts[i];
        (void)memset(cmdt_p, 0x00, sizeof(vdp1_cmdt_t));
        cmdt_p->cmd_xa = 0;
        cmdt_p->cmd_ya = i;
        cmdt_p->cmd_size = 0x2C01;
        cmdt_p->cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*i+704) / 8;
        vdp1_cmdt_param_color_mode4_set(cmdt_p, dummy_bank);
        vdp1_cmdt_param_color_bank_set(cmdt_p, dummy_bank);
        cmdt_p->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now
    }
    //filling another 224 quads
    for (int i=0;i<224;i++)
    {
        cmdt_p = &_svin_tapestry_cmdt_list_2->cmdts[i+224];
        (void)memset(cmdt_p, 0x00, sizeof(vdp1_cmdt_t));
        cmdt_p->cmd_xa = 352;
        cmdt_p->cmd_ya = i;
        cmdt_p->cmd_size = 0x2C01;
        cmdt_p->cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*i+1056) / 8;
        vdp1_cmdt_param_color_mode4_set(cmdt_p, dummy_bank);
        vdp1_cmdt_param_color_bank_set(cmdt_p, dummy_bank);
        cmdt_p->cmd_pmod |= 0x08C0; //enabling ECD and SPD manually for now
    }

    cmdt_p = &_svin_tapestry_cmdt_list_2->cmdts[448];
    (void)memset(cmdt_p, 0x00, sizeof(vdp1_cmdt_t));
    vdp1_cmdt_end_set(cmdt_p);

    _svin_tapestry_cmdt_list_2->count = 449;

    vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_2, 32*480, NULL, NULL); 
    vdp_sync();  
}

void 
_svin_tapestry_load_position(iso9660_filelist_t *_filelist, char *filename, int position)
{
    //locating the the tapestry start FAD first
    iso9660_filelist_entry_t *file_entry;
    _svin_tapestry_pack_fad = 0;
    _svin_tapestry_framebuffer_start = 0;
    _svin_tapestry_data_start = 0;

    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    uint8_t tmp_sector[2048];
    uint16_t *tmp_sector_16 = (uint16_t *)tmp_sector;

    for (unsigned int i = 0; i < _filelist->entries_count; i++)
    {
        file_entry = &(_filelist->entries[i]);
        if (strcmp(file_entry->name, filename) == 0)
        {
            _svin_tapestry_pack_fad = file_entry->starting_fad;
            //get filesize to load palette
            //reading 1st block to find out number of blocks
            _svin_cd_block_sector_read(_svin_tapestry_pack_fad, tmp_sector);
            //getting size and number of entries
            int iSizeY = tmp_sector_16[32+3];
            assert(iSizeY > 0);
            _svin_cd_block_sector_read(_svin_tapestry_pack_fad + 1 + iSizeY/2 + 0, tmp_sector);
            _svin_set_palette(0, tmp_sector);
        }
    }
    assert(_svin_tapestry_pack_fad > 0);

    //now that we know the FAD, load 224 blocks from the position start
    for (int i=0;i<224;i++)
    {
        _svin_cd_block_sector_read(_svin_tapestry_pack_fad + 1 + position + i, tmp_sector);
        memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + i*1408), tmp_sector, 1408);
    }
}

void 
_svin_tapestry_move_up()
{
    uint8_t tmp_sector[2048];
    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    //use previously opened tapestry
    if (_svin_tapestry_pack_fad != 0)
    {
        if (0 == _svin_tapestry_data_start)
            return;

        //vdp_sync();

        //load the sector
        if ( 1 == _svin_tapestry_movement_active)
        {
            //already moving up, the sector is requested, we only need to fetch it
            _svin_cd_block_sector_read_process(tmp_sector);
        }
        else
        {
            //starting movement, first flushing whatever was prefetched
            _svin_cd_block_sector_read_flush(tmp_sector);
            //now doing a full read
            _svin_cd_block_sector_read(_svin_tapestry_pack_fad + 1 + _svin_tapestry_data_start + 0, tmp_sector);
        }
        memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + ((_svin_tapestry_framebuffer_start+0)%250)*1408), tmp_sector, 1408);
        //reindex the positions in framebuffer
        for (int i = 0; i<224;i++)
        {
            int framebuffer_pos = (_svin_tapestry_framebuffer_start+i)%250;
            _svin_tapestry_cmdt_list_1->cmdts[i    ].cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*framebuffer_pos+0) / 8;
            _svin_tapestry_cmdt_list_1->cmdts[i+224].cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*framebuffer_pos+352) / 8;
            _svin_tapestry_cmdt_list_2->cmdts[i    ].cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*framebuffer_pos+704) / 8;
            _svin_tapestry_cmdt_list_2->cmdts[i+224].cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*framebuffer_pos+1056) / 8;
        }
        vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_1, 32*16, NULL, NULL); 
        vdp_sync();  
        vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_2, 32*480, NULL, NULL); 
        vdp_sync();  
        _svin_tapestry_framebuffer_start--;
        _svin_tapestry_data_start--;
        if (_svin_tapestry_framebuffer_start < 0)
            _svin_tapestry_framebuffer_start += 250;
        //prefetching data for consecutive movement
        _svin_cd_block_sector_read_request(_svin_tapestry_pack_fad + 1 + _svin_tapestry_data_start + 0); 
        _svin_tapestry_movement_active = 1;
    }
}

void 
_svin_tapestry_move_down()
{
    uint8_t tmp_sector[2048];
    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    //use previously opened tapestry
    if (_svin_tapestry_pack_fad != 0)
    {
        //vdp_sync();
        //load the sector
        if ( -1 == _svin_tapestry_movement_active)
        {
            //already moving down, the sector is requested, we only need to fetch it
            _svin_cd_block_sector_read_process(tmp_sector);
        }
        else
        {
            //starting movement, first flushing whatever was prefetched
            _svin_cd_block_sector_read_flush(tmp_sector);
            //now doing a full read
            _svin_cd_block_sector_read(_svin_tapestry_pack_fad + 1 + _svin_tapestry_data_start + 224, tmp_sector);
        }
        memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + ((_svin_tapestry_framebuffer_start+224)%250)*1408), tmp_sector, 1408);
        //reindex the positions in framebuffer
        for (int i = 0; i<224;i++)
        {
            int framebuffer_pos = (_svin_tapestry_framebuffer_start+i)%250;
            _svin_tapestry_cmdt_list_1->cmdts[i    ].cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*framebuffer_pos+0) / 8;
            _svin_tapestry_cmdt_list_1->cmdts[i+224].cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*framebuffer_pos+352) / 8;
            _svin_tapestry_cmdt_list_2->cmdts[i    ].cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*framebuffer_pos+704) / 8;
            _svin_tapestry_cmdt_list_2->cmdts[i+224].cmd_srca = ((int)vdp1_vram_partitions.texture_base-VDP1_VRAM(0)+1408*framebuffer_pos+1056) / 8;
        }
        vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_1, 32*16, NULL, NULL); 
        vdp_sync();  
        vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_2, 32*480, NULL, NULL); 
        vdp_sync();  
        _svin_tapestry_framebuffer_start++;
        _svin_tapestry_data_start++;
        if (_svin_tapestry_framebuffer_start >= 250)
            _svin_tapestry_framebuffer_start = 0;
        //prefetching data for consecutive movement
        _svin_cd_block_sector_read_request(_svin_tapestry_pack_fad + 1 + _svin_tapestry_data_start + 224); 
        _svin_tapestry_movement_active = -1;
    }
}

//---------------------------------------------- Actor stuff ----------------------------------------------------

//Actors sprites are stored in VDP1 VRAM. When new actor is requested, part of the actor pack is loaded from CD.
//The actor pack includes different zooms, poses, costumes and emotions.
//Since VRAM is very limited, only a single zoom, single costume and single pose are loaded from the pack, with entire set of corresponding emotions.
//One should specify an actor, zoom, pose and a costume via global indexes.

/*void
_svin_actor_load(iso9660_filelist_t * _filelist, char * actor_filename, int actor_id, int pose_id, int costume_id)
{
    //-------------- Getting FAD and index for background pack binary -------------------
    iso9660_filelist_entry_t *file_entry;
    _svin_background_pack_fad = 0;
    for (unsigned int i =0; i< _filelist->entries_count; i++)
    {
        file_entry = &(_filelist->entries[i]);
        if (strcmp(file_entry->name,actor_filename) == 0)
        {
            _svin_actor_pack_fad[actor_id] = file_entry->starting_fad;
            //allocating temporary buf for 1 sector
            uint8_t tmp_sector[2048];
            uint16_t * tmp_sector_16 = (uint16_t * )tmp_sector;
            //reading 1st block to find out number of blocks
            _svin_cd_block_sector_read(_svin_background_pack_fad, tmp_sector);
            //getting size and number of entries
            assert(64 == tmp_sector_16[5]); //non-64-byte entries not supported
            _svin_background_files_number = tmp_sector_16[4];
            //allocating array for background pack index
            assert(_svin_background_files_number < 200); //hard limit for backgound number
            int blocks_for_index = (_svin_background_files_number)/32+1;
            _svin_background_index = malloc(blocks_for_index*2048);
            //reading
            _svin_cd_block_multiple_sectors_read(_svin_background_pack_fad, blocks_for_index, _svin_background_index);
        }
    }
    assert(_svin_background_pack_fad > 0);
}*/

void _svin_actor_debug_load_index(iso9660_filelist_t *_filelist)
{
    //-------------- Getting FAD and index for actor_debug pack binary -------------------
    iso9660_filelist_entry_t *file_entry;
    _svin_actor_debug_pack_fad = 0;
    for (unsigned int i = 0; i < _filelist->entries_count; i++)
    {
        file_entry = &(_filelist->entries[i]);
        if (strcmp(file_entry->name, "SL.PAK") == 0)
        {
            _svin_actor_debug_pack_fad = file_entry->starting_fad;
            //allocating temporary buf for 1 sector
            uint8_t tmp_sector[2048];
            uint16_t *tmp_sector_16 = (uint16_t *)tmp_sector;
            //reading 1st block to find out number of blocks
            _svin_cd_block_sector_read(_svin_actor_debug_pack_fad, tmp_sector);
            //getting size and number of entries
            assert(64 == tmp_sector_16[5]); //non-64-byte entries not supported
            _svin_actor_debug_files_number = tmp_sector_16[4];
            //allocating array for actor_debug pack index
            assert(_svin_actor_debug_files_number < 200); //hard limit for actor_debug number
            int blocks_for_index = (_svin_actor_debug_files_number) / 32 + 1;
            _svin_actor_debug_index = malloc(blocks_for_index * 2048);
            //reading
            _svin_cd_block_multiple_sectors_read(_svin_actor_debug_pack_fad, blocks_for_index, _svin_actor_debug_index);
        }
    }
    assert(_svin_actor_debug_pack_fad > 0);
}

//starting with a simple texture - load a single sprite from the pack and copy it into VDP1 ram
void _svin_actor_debug_load_test(iso9660_filelist_t *_filelist, char *filename, int actor_id)
{
    //-------------- Getting FAD and index for actor_debug pack binary -------------------
    iso9660_filelist_entry_t *file_entry;
    assert(actor_id < 3); //only 3 actors are supported for now
    for (unsigned int i = 0; i < _filelist->entries_count; i++)
    {
        file_entry = &(_filelist->entries[i]);
        if (strcmp(file_entry->name, filename) == 0)
        {
            //allocating temporary buf for 1 sector
            uint8_t tmp_sector[2048];
            uint16_t *tmp_sector_16 = (uint16_t *)tmp_sector;
            //reading 1st block to find out number of blocks
            _svin_cd_block_sector_read(file_entry->starting_fad, tmp_sector);
            //getting size and number of entries
            assert(64 == tmp_sector_16[5]); //non-64-byte entries not supported
            //assert(1 == tmp_sector_16[4]);  //working with a single-file sprite packs for now
            //fetch actor geometry
            /*_svin_actor_left[actor_id] = tmp_sector_16[32+64*actor_id];
            _svin_actor_top[actor_id] = tmp_sector_16[33+64*actor_id];
            _svin_actor_sizex[actor_id] = tmp_sector_16[34+64*actor_id];
            _svin_actor_sizey[actor_id] = tmp_sector_16[35+64*actor_id];*/
            _svin_actor_left[0] = tmp_sector_16[32+64*0];
            _svin_actor_top[0] = tmp_sector_16[33+64*0];
            _svin_actor_sizex[0] = tmp_sector_16[34+64*0];
            _svin_actor_sizey[0] = tmp_sector_16[35+64*0];
            //calculating required blocks
            //int blocks_for_sprite = (_svin_actor_sizex[actor_id] * _svin_actor_sizey[actor_id] - 1) / 2048 + 1;
            int blocks_for_sprite = (_svin_actor_sizex[0] * _svin_actor_sizey[0] - 1) / 2048 + 1;
            //allocating mem
            uint8_t *buffer = malloc(blocks_for_sprite * 2048);
            assert((int)(buffer) > 0);
            //allocate memory for cram
            uint8_t *palette = malloc(2048);
            assert((int)(palette) > 0);
            
            //setting cycle patterns for cpu access
            _svin_set_cycle_patterns_cpu();
            
            uint16_t *_svin_actor_debug_index_16 = (uint16_t *)_svin_actor_debug_index;//[actor_id]
            //uint16_t _current_sector = _svin_actor_debug_index_16[actor_id * 32 + 36];
            uint16_t _current_sector = _svin_actor_debug_index_16[0 * 32 + 36];

            //this is a fast version for hacked yaul
            //reading whole block
            _svin_cd_block_multiple_sectors_read(_svin_actor_debug_pack_fad + _current_sector, blocks_for_sprite, buffer);
            //memcpy((uint8_t *)(_SVIN_NBG0_CHPNDR_START + 0x20000 * actor_id), buffer, _svin_actor_sizex[actor_id] * _svin_actor_sizey[actor_id]);
            memcpy((uint8_t *)(_SVIN_NBG0_CHPNDR_START + 0x20000 * actor_id), buffer, _svin_actor_sizex[0] * _svin_actor_sizey[0]);

            //update names
            //starting with plane 0
            int *_pointer32 = (int *)_SVIN_NBG0_PNDR_START;
            int size_x = _svin_actor_sizex[0]/16;
            for (int y = 0; y < (_svin_actor_sizey[0]/16); y++)
            {
                for (int x = 0; x < (_svin_actor_sizex[0]/16); x++)
                {
                    _pointer32[y*32 + x + 32*8 + 5 + 10*actor_id] = 0x10100000 + (y*size_x + x) * 8; //palette 1, transparency
                }
            }
            //plane 1 goes next
            /*for (int y = 0; y < 28; y++)
            {
                int iOffset = 32 * 32 + y * 32;
                //int iOffset2 = 32 * 28 + y * 12;
                for (int x = 0; x < 12; x++)
                {
                    _pointer32[iOffset + x] = 0x10100000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 1, transparency on; //(iOffset2 + x) * 8;
                }
            }*/


            //read palette
            _svin_cd_block_sector_read(_svin_actor_debug_pack_fad + _current_sector + blocks_for_sprite, palette);
            _svin_set_palette(1+actor_id, palette);

            //setting cycle patterns for nbg access
            _svin_set_cycle_patterns_nbg();

            free(palette);
            free(buffer);
        }
    }

}


void _svin_vblank_out_handler(void *work __unused)
{
    if (0==_svin_init_done)
        return;

    //smpc_peripheral_intback_issue();
    uint8_t * p = (uint8_t *)VDP1_VRAM(0); 

    if (VDP2_TVMD_TV_FIELD_SCAN_ODD == vdp2_tvmd_field_scan_get())
        //vdp1_cmdt_jump_assign(&_svin_cmdt_list->cmdts[_SVIN_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX], _SVIN_VDP1_ORDER_LOCAL_COORDS_A_INDEX * 4);
        p[3]=0x1C;
    else
        //vdp1_cmdt_jump_assign(&_svin_cmdt_list->cmdts[_SVIN_VDP1_ORDER_SYSTEM_CLIP_COORDS_INDEX], _SVIN_VDP1_ORDER_LOCAL_COORDS_B_INDEX * 4);
        p[3]=0x04;

    //vdp1_sync_cmdt_list_put(_svin_cmdt_list, 0, NULL, NULL);
    
    smpc_peripheral_intback_issue();
}

void _svin_wait_for_key_press_and_release()
{
    bool bPressed = false;
    while (false == bPressed)
    {
        smpc_peripheral_process();
        smpc_peripheral_digital_port(1, &_digital);
        if (_digital.pressed.raw != 0)
            bPressed = true;
    }
    _svin_delay(15);//debounce
    while (true == bPressed)
    {
        smpc_peripheral_process();
        smpc_peripheral_digital_port(1, &_digital);
        if (_digital.pressed.raw == 0)
            bPressed = false;
    }
}