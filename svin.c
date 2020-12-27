#include <yaul.h>

#include <tga.h>

#include <svin.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern int cd_block_multiple_sectors_read(uint32_t fad, uint32_t number, uint8_t *output_buffer);

fad_t _svin_background_pack_fad;
uint8_t *_svin_background_pack_index;
uint16_t _svin_background_files_number;

void
_svin_delay(int milliseconds)
{
    //delay
    volatile int dummy=0;
    for (dummy = 0; dummy < 3000*milliseconds; dummy++)
        ;
}

void 
_svin_background_fade_to_black_step()
{
    uint16_t *my_vdp2_cram = (uint16_t *)VDP2_VRAM_ADDR(8, 0x00000);
    uint8_t r,g,b;
    for (int i=0;i<256;i++)
    {
        b = (my_vdp2_cram[i]&0x7C00)>>10;
        g = (my_vdp2_cram[i]&0x03E0)>>5;
        r = (my_vdp2_cram[i]&0x001F)>>0;
        r--;b--;g--;
        if (r==0xFF) r = 0;
        if (g==0xFF) g = 0;
        if (b==0xFF) b = 0;
        my_vdp2_cram[i] = ( (b<<10) |
                            (g<<5) |
                            (r<<0) );
    }
}

void 
_svin_background_fade_to_black()
{
    for (int fade=0;fade<32;fade++)
    {
        _svin_background_fade_to_black_step();
        _svin_delay(30);
    }
}

void 
_svin_background_set_palette(int number, uint8_t * pointer)
{
    uint16_t *my_vdp2_cram = (uint16_t *)VDP2_VRAM_ADDR(8, 0x200 * number);
    for (int i=0;i<256;i++)
    {
        my_vdp2_cram[i] = ( ((pointer[i*3+2]&0xF8)<<7) |
                            ((pointer[i*3+1]&0xF8)<<2) |
                            ((pointer[i*3+0]&0xF8)>>3) );
    }
}

void 
_svin_background_clear_palette(int number)
{
    uint16_t *my_vdp2_cram = (uint16_t *)VDP2_VRAM_ADDR(8, 0x200 * number);
    for (int i=0;i<256;i++)
    {
        my_vdp2_cram[i] = 0;
    }
}

void
_svin_set_cycle_patterns_cpu()
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

void
_svin_set_cycle_patterns_nbg()
{
        //swithcing everything to NBG accesses, CPU can't write data anymore
        //these settings are proved to work on real hardware, please do the test if you plan to change them

        struct vdp2_vram_cycp vram_cycp;

        vram_cycp.pt[0].t0 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[0].t1 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[0].t2 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[0].t3 = VDP2_VRAM_CYCP_PNDR_NBG0;
        vram_cycp.pt[0].t4 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[0].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[0].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[0].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

        vram_cycp.pt[1].t0 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[1].t1 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[1].t2 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[1].t3 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[1].t4 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[1].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[1].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[1].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

        vram_cycp.pt[2].t0 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[2].t1 = VDP2_VRAM_CYCP_CHPNDR_NBG0;
        vram_cycp.pt[2].t2 = VDP2_VRAM_CYCP_PNDR_NBG1;
        vram_cycp.pt[2].t3 = VDP2_VRAM_CYCP_PNDR_NBG0;
        vram_cycp.pt[2].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[2].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[2].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[2].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

        vram_cycp.pt[3].t0 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
        vram_cycp.pt[3].t1 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
        vram_cycp.pt[3].t2 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
        vram_cycp.pt[3].t3 = VDP2_VRAM_CYCP_CHPNDR_NBG1;
        vram_cycp.pt[3].t4 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[3].t5 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[3].t6 = VDP2_VRAM_CYCP_NO_ACCESS;
        vram_cycp.pt[3].t7 = VDP2_VRAM_CYCP_NO_ACCESS;

        vdp2_vram_cycp_set(&vram_cycp);
        vdp_sync();

        //enable transparency for NBG1 over NBG0 (might not work with sprites between
        MEMORY_WRITE(16, VDP2(CCCTL), 0x0002); //enable cc for NBG1
        MEMORY_WRITE(16, VDP2(CCRNA), 0x0C00); //enable cc for NBG1
}

void
_svin_init(iso9660_filelist_t * _filelist)
{
    int * _pointer32;

	//clearing vdp2
	vdp2_tvmd_display_clear();

	//setup nbg0
	struct vdp2_scrn_cell_format format;
	memset(&format, 0x00, sizeof(format));

	format.scroll_screen = VDP2_SCRN_NBG0;
	format.cc_count = VDP2_SCRN_CCC_PALETTE_256;
	format.character_size = (2*2);
	format.pnd_size = 2;
	format.auxiliary_mode = 1;
	format.cp_table = 0;
	format.color_palette = 0;
	format.plane_size = (2*1);
	format.sf_type = VDP2_SCRN_SF_TYPE_NONE;
	format.sf_code = VDP2_SCRN_SF_CODE_A;
	format.sf_mode = 0;
	format.map_bases.plane_a = _SVIN_NBG0_PNDR_START;

	vdp2_scrn_cell_format_set(&format);
	vdp2_scrn_priority_set(VDP2_SCRN_NBG0, 3);
	vdp2_scrn_display_set(VDP2_SCRN_NBG0, false);
	vdp2_cram_mode_set(0);

	//setup nbg1
	format.scroll_screen = VDP2_SCRN_NBG1;
	format.cc_count = VDP2_SCRN_CCC_PALETTE_256;
	format.character_size = (2*2);
	format.pnd_size = 2;
	format.auxiliary_mode = 1;
	format.cp_table = 0;
	format.color_palette = 0;
	format.plane_size = (2*1);
	format.sf_type = VDP2_SCRN_SF_TYPE_COLOR_CALCULATION;
	format.sf_code = VDP2_SCRN_SF_CODE_A;
	format.sf_mode = 0;
	format.map_bases.plane_a = _SVIN_NBG1_PNDR_START;

	vdp2_scrn_cell_format_set(&format);
	vdp2_scrn_priority_set(VDP2_SCRN_NBG1, 5);
	vdp2_scrn_display_set(VDP2_SCRN_NBG1, true);

	//setting cycle patterns for cpu access
	_svin_set_cycle_patterns_cpu();

    vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_DOUBLE, VDP2_TVMD_HORZ_HIRESO_B, VDP2_TVMD_VERT_224); //704x448 works
    vdp2_tvmd_display_set();
    vdp_sync();

    color_rgb1555_t bs_color;
    bs_color = COLOR_RGB1555(0, 0, 0, 0);
    vdp2_scrn_back_screen_color_set(VDP2_VRAM_ADDR(3, 0x01FFFE),bs_color);

    //-------------- setup pattern names -------------------

    //writing pattern names for nbg0
	//starting with plane 0
	_pointer32 = (int *)_SVIN_NBG0_PNDR_START;
	for (int y = 0; y < 28; y++)
    {
        int iOffset = y*32;
        for (int x = 0; x < 32; x++)
        {
            _pointer32[iOffset+x] = (iOffset+x)*8;
        }
    }
    //plane 1 goes next
	for (int y = 0; y < 28; y++)
    {
        int iOffset = 32*32 + y*32;
        int iOffset2 = 32*28 + y*12;
        for (int x = 0; x < 12; x++)
        {
            _pointer32[iOffset+x] = (iOffset2+x)*8;
        }
    }

	//writing pattern names for nbg1
	//nbg1  is mostly transparent, so fill with that one
	_pointer32 = (int *)_SVIN_NBG1_PNDR_START;
    for (unsigned int i=0;i<_SVIN_NBG1_PNDR_SIZE/sizeof(int);i++)
    {
        _pointer32[i] = 0x10100000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX; //palette 1, transparency on
    }
    //writing semi-transparent characters where the dialog box should go, plane 0 first
    for (int y = 22; y < 27; y++)
    {
        int iOffset = y*32;
        for (int x = 4; x < 32; x++)
        {
            _pointer32[iOffset+x] = 0x10100000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX + _SVIN_CHARACTER_UNITS*1; //palette 1, transparency on
        }
    }
    //now plane 1
    for (int y = 22; y < 27; y++)
    {
        int iOffset = 32*32 + y*32;
        for (int x = 0; x < 8; x++)
        {
            _pointer32[iOffset+x] = 0x10100000 + _SVIN_NBG1_CHPNDR_SPECIALS_INDEX + _SVIN_CHARACTER_UNITS*1; //palette 1, transparency on
        }
    }

    //-------------- setup character pattern names -------------------

	//clearing character pattern names data for nbg0
	_pointer32 = (int *)_SVIN_NBG0_CHPNDR_START;
    for (unsigned int i=0;i<_SVIN_NBG0_CHPNDR_SIZE/sizeof(int);i++)
    {
        _pointer32[i] = 0;
    }

    //clearing character pattern names data for nbg1
	_pointer32 = (int *)_SVIN_NBG1_CHPNDR_START;
    for (unsigned int i=0;i<_SVIN_NBG1_CHPNDR_SIZE/sizeof(int);i++)
    {
        _pointer32[i] = 0;
    }

    //setting up "transparent" character for nbg1
    _pointer32 = (int *)_SVIN_NBG1_CHPNDR_SPECIALS_ADDR;
    for (unsigned int i=0;i<_SVIN_CHARACTER_BYTES/sizeof(int);i++)
    {
        _pointer32[i] = 0;
    }

    //setting up "semi-transparent" character for nbg1
    _pointer32 = (int *)(_SVIN_NBG1_CHPNDR_SPECIALS_ADDR+_SVIN_CHARACTER_BYTES);
    for (unsigned int i=0;i<_SVIN_CHARACTER_BYTES/sizeof(int);i++)
    {
        _pointer32[i] = 0x7F7F7F7F;
    }

    //-------------- setup palettes -------------------

    //setup default palettes
    uint8_t temp_pal[3*256];
	for (int i=0;i<256;i++)
	{ 
        temp_pal[i*3] = i;//( (i/8)*0x0421 );//grayscale gradient
        temp_pal[i*3+1] = i;//( (i/8)*0x0421 );//grayscale gradient
        temp_pal[i*3+2] = i;//( (i/8)*0x0421 );//grayscale gradient
    }
    for (int pal=0;pal<4;pal++)
        _svin_background_set_palette(pal,temp_pal);

	//setting cycle patterns for nbg access
	_svin_set_cycle_patterns_nbg();
    
    //-------------- Getting FAD and index for background pack binary -------------------
    iso9660_filelist_entry_t *file_entry;
    _svin_background_pack_fad = 0;
    for (unsigned int i =0; i< _filelist->entries_count; i++)
    {
        file_entry = &(_filelist->entries[i]);
        if (strcmp(file_entry->name,"BG.PAK") == 0)
        {
            _svin_background_pack_fad = file_entry->starting_fad;
            //allocating temporary buf for 1 sector
            uint8_t tmp_sector[2048];
            uint16_t * tmp_sector_16 = (uint16_t * )tmp_sector;
            //reading 1st block to find out number of blocks
            cd_block_sector_read(_svin_background_pack_fad, tmp_sector);
            //getting size and number of entries
            assert(64 == tmp_sector_16[5]); //non-64-byte entries not supported
            _svin_background_files_number = tmp_sector_16[4];
            //allocating array for background pack index
            assert(_svin_background_files_number < 200); //hard limit for backgound number
            int blocks_for_index = (_svin_background_files_number)/32+1;
            _svin_background_pack_index = malloc(blocks_for_index*2048);
            //reading
            cd_block_multiple_sectors_read(_svin_background_pack_fad, blocks_for_index, _svin_background_pack_index);
        }
    }
    assert(_svin_background_pack_fad > 0);
}

void 
_svin_background_set_by_index(int index)
{
    if (index < 0)
        return;

    //allocate memory for 77 sectors
    uint8_t * buffer = malloc(77*2048);
    assert ((int)(buffer) > 0);
    //allocate memory for cram
    uint8_t * palette = malloc(2048);
    assert ((int)(palette) > 0);

    //set zero palette to hide loading
    _svin_background_clear_palette(0);
    
    //setting cycle patterns for cpu access
	_svin_set_cycle_patterns_cpu();

    uint16_t * _svin_background_pack_index_16 = (uint16_t*)_svin_background_pack_index;
    uint16_t _current_sector = _svin_background_pack_index_16[index*32+36];

    //this is a stock slow version for unhacked yaul
    //for (int sector=0;sector<154;sector++)
    //{
    //    cd_block_sector_read(_svin_background_pack_fad + 4 + sector + index*156, (uint8_t *) (_SVIN_NBG0_CHPNDR_START+sector*2048) );
    //}
    //this is a fast version for hacked yaul
    cd_block_multiple_sectors_read(_svin_background_pack_fad + _current_sector, 77, buffer);
    memcpy((uint8_t *) (_SVIN_NBG0_CHPNDR_START+0*2048),buffer,2048*77);
    cd_block_multiple_sectors_read(_svin_background_pack_fad + _current_sector + 77, 77, buffer);
    memcpy((uint8_t *) (_SVIN_NBG0_CHPNDR_START+77*2048),buffer,2048*77);

    //read palette
     cd_block_sector_read(_svin_background_pack_fad + _current_sector + 154, palette );
    _svin_background_set_palette(0,palette);

    //setting cycle patterns for nbg access
    _svin_set_cycle_patterns_nbg();

    free (palette);
    free (buffer);
}

void 
_svin_background_set(char *name)
{   
    //searching for a name
    int iFoundIndex = -1;
    for (int i=0;i<128;i++)
    {
        if ( 0 == strcmp(name,(char *)&(_svin_background_pack_index[i*64+74])) )
        {
            iFoundIndex = i;
        }
    }

    if (iFoundIndex < 0)
        return;
    _svin_background_set_by_index(iFoundIndex);
}

void 
_svin_background_update(char *name)
{   
    //searching for a name
    int iFoundIndex = -1;
    for (int i=0;i<128;i++)
    {
        if ( 0 == strcmp(name,(char *)&(_svin_background_pack_index[i*64+74])) )
        {
            iFoundIndex = i;
        }
    }

    if (iFoundIndex < 0)
        return;
    _svin_background_update_by_index(iFoundIndex);
}

void 
_svin_background_update_by_index(int index)
{
    if (index < 0)
        return;

    //allocate memory for 154 sectors
    uint8_t * buffer = malloc(154*2048);
    assert ((int)(buffer) > 0);
    //allocate memory for cram
    uint8_t * palette = malloc(2048);
    assert ((int)(palette) > 0);

    uint16_t * _svin_background_pack_index_16 = (uint16_t*)_svin_background_pack_index;
    uint16_t _current_sector = _svin_background_pack_index_16[index*32+36];

    //read next image while fading-to-black current one
    for (int i=0;i<14;i++)
    {
        cd_block_multiple_sectors_read(_svin_background_pack_fad + _current_sector + i*11, 11, &(buffer[11*2048*i]));
        _svin_background_fade_to_black_step();
    }
    
    //continue fading-to-black
    for (int i=0;i<18;i++)
    {
        _svin_delay(50);
        _svin_background_fade_to_black_step();
    }

    //setting cycle patterns for cpu access
	_svin_set_cycle_patterns_cpu();

    //copy preloaded pattern names
    memcpy((uint8_t *) (_SVIN_NBG0_CHPNDR_START),buffer,2048*154);

    //read palette
     cd_block_sector_read(_svin_background_pack_fad + _current_sector + 154, palette );
    _svin_background_set_palette(0,palette);

    //setting cycle patterns for nbg access
    _svin_set_cycle_patterns_nbg();

    free (buffer);
    free (palette);
}

void 
_svin_background_clear()
{
    //set zero palette
    _svin_background_clear_palette(0);
}

