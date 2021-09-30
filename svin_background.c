#include <yaul.h>
#include <vdp1/vram.h>
#include <svin.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <cd-block_multiread.h>

extern int cd_block_multiple_sectors_read(uint32_t fad, uint32_t number, uint8_t *output_buffer);

    #define _svin_cd_block_multiple_sectors_read cd_block_multiple_sectors_read
    #define _svin_cd_block_sector_read cd_block_sector_read
    #define _svin_cd_block_sector_read_request cd_block_sector_read_request
    #define _svin_cd_block_sector_read_process cd_block_sector_read_process
    #define _svin_cd_block_sector_read_flush cd_block_sector_read_flush

fad_t _svin_background_pack_fad;
uint8_t *_svin_background_index;
uint16_t _svin_background_files_number;
vdp1_cmdt_list_t *_svin_cmdt_list;
vdp1_cmdt_list_t *_svin_tapestry_cmdt_list_1;
vdp1_cmdt_list_t *_svin_tapestry_cmdt_list_2;
extern uint8_t _svin_init_done;

void _svin_background_fade_to_black_step()
{
    uint16_t *my_vdp2_cram = (uint16_t *)VDP2_VRAM_ADDR(8, 0x00000);
    uint8_t r, g, b;
    for (int i = 0; i < 256; i++)
    {
        b = (my_vdp2_cram[i] & 0x7C00) >> 10;
        g = (my_vdp2_cram[i] & 0x03E0) >> 5;
        r = (my_vdp2_cram[i] & 0x001F) >> 0;
        r--;
        b--;
        g--;
        if (r == 0xFF)
            r = 0;
        if (g == 0xFF)
            g = 0;
        if (b == 0xFF)
            b = 0;
        my_vdp2_cram[i] = ((b << 10) |
                           (g << 5) |
                           (r << 0));
    }
}

void _svin_background_fade_to_black()
{
    for (int fade = 0; fade < 32; fade++)
    {
        _svin_background_fade_to_black_step();
        _svin_delay(30);
    }
}

void _svin_background_load_index(char * filename)
{
    int i;
    //-------------- Getting FAD and index for background pack binary -------------------
    assert(true == _svin_filelist_search(filename,&_svin_background_pack_fad,&i));
    //allocating temporary buf for 1 sector
    uint8_t tmp_sector[2048];
    uint16_t *tmp_sector_16 = (uint16_t *)tmp_sector;
    //reading 1st block to find out number of blocks
    _svin_cd_block_sector_read(_svin_background_pack_fad, tmp_sector);
    //getting size and number of entries
    assert(64 == tmp_sector_16[5]); //non-64-byte entries not supported
    _svin_background_files_number = tmp_sector_16[4];
    //allocating array for background pack index
    assert(_svin_background_files_number < 200); //hard limit for backgound number
    int blocks_for_index = (_svin_background_files_number) / 32 + 1;
    _svin_background_index = malloc(blocks_for_index * 2048);
    //reading
    _svin_cd_block_multiple_sectors_read(_svin_background_pack_fad, blocks_for_index, _svin_background_index);
}

void _svin_background_set_by_index(int index)
{
    if (index < 0)
        return;

    //_svin_background_set_by_index_half(index,0,0);
    //_svin_background_set_by_index_half(index,1,1);

    //allocate memory for 77 sectors
    uint8_t *buffer = malloc(77 * 2048);
    assert((int)(buffer) > 0);
    //allocate memory for cram
    uint8_t *palette = malloc(2048);
    assert((int)(palette) > 0);

    //set zero palette to hide loading
    _svin_clear_palette(0);

    //setting cycle patterns for cpu access
    _svin_set_cycle_patterns_cpu();

    uint16_t *_svin_background_index_16 = (uint16_t *)_svin_background_index;
    uint16_t _current_sector = _svin_background_index_16[index * 32 + 36];

    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    //reading first half of the background
    _svin_cd_block_multiple_sectors_read(_svin_background_pack_fad + _current_sector, 77, buffer);
    //memset((uint8_t *)(_SVIN_NBG0_CHPNDR_START + 0 * 2048), 0x00, 2048 * 77);
    memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + 0 * 2048), buffer, 2048 * 77);

    //reading second half of the background
    _svin_cd_block_multiple_sectors_read(_svin_background_pack_fad + _current_sector + 77, 77, buffer);
    //memset((uint8_t *)(_SVIN_NBG0_CHPNDR_START + 77 * 2048), 0x00, 2048 * 77);
    memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + 77 * 2048), buffer, 2048 * 77);

    //reading first half of the background
    /*_svin_cd_block_multiple_sectors_read(_svin_background_pack_fad + _current_sector, 77, buffer);
    //copying half
    for (int i=0;i<224;i++)
    {
        memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + i*352 + 352*336*0), buffer + i*352, 352);
        memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + i*352 + 352*336*1), buffer + i*352 + 352*224, 352);
    }

    //reading second half of the background
    _svin_cd_block_multiple_sectors_read(_svin_background_pack_fad + _current_sector + 77, 77, buffer);
    for (int i=0;i<224;i++)
    {
        memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + i*352 + 352*336*2), buffer + i*352, 352);
        memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + i*352 + 352*336*3), buffer + i*352 + 352*224, 352);
    }*/

    //read palette
    _svin_cd_block_sector_read(_svin_background_pack_fad + _current_sector + 154, palette);
    _svin_set_palette(0, palette);

    //setting cycle patterns for nbg access
    _svin_set_cycle_patterns_nbg();

    free(palette);
    free(buffer);
}

void _svin_background_set(char *name)
{
    //searching for a name
    int iFoundIndex = -1;
    for (int i = 0; i < 128; i++)
    {
        if (0 == strcmp(name, (char *)&(_svin_background_index[i * 64 + 74])))
        {
            iFoundIndex = i;
        }
    }

    if (iFoundIndex < 0)
        return;
    _svin_background_set_by_index(iFoundIndex);
}

void _svin_background_update(char *name)
{
    //searching for a name
    int iFoundIndex = -1;
    for (int i = 0; i < 128; i++)
    {
        if (0 == strcmp(name, (char *)&(_svin_background_index[i * 64 + 74])))
        {
            iFoundIndex = i;
        }
    }

    if (iFoundIndex < 0)
        return;
    _svin_background_update_by_index(iFoundIndex);
}

void _svin_background_update_by_index(int index)
{
    if (index < 0)
        return;

    //allocate memory for 154 sectors
    uint8_t *buffer = malloc(154 * 2048);
    assert((int)(buffer) > 0);
    //allocate memory for cram
    uint8_t *palette = malloc(2048);
    assert((int)(palette) > 0);

    uint16_t *_svin_background_index_16 = (uint16_t *)_svin_background_index;
    uint16_t _current_sector = _svin_background_index_16[index * 32 + 36];

    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    //read next image while fading-to-black current one
    for (int i = 0; i < 14; i++)
    {
        _svin_cd_block_multiple_sectors_read(_svin_background_pack_fad + _current_sector + i * 11, 11, &(buffer[11 * 2048 * i]));
        _svin_background_fade_to_black_step();
    }

    //continue fading-to-black
    for (int i = 0; i < 18; i++)
    {
        _svin_delay(50);
        _svin_background_fade_to_black_step();
    }

    //copy preloaded pattern names
    //memcpy((uint8_t *)(_SVIN_NBG0_CHPNDR_START), buffer, 2048 * 154);
    memcpy((uint8_t *)(vdp1_vram_partitions.texture_base), buffer, 2048 * 154);

    //read palette
    _svin_cd_block_sector_read(_svin_background_pack_fad + _current_sector + 154, palette);
    _svin_set_palette(0, palette);

    free(buffer);
    free(palette);
}

void _svin_background_clear()
{
    //set zero palette
    _svin_clear_palette(0);
}
