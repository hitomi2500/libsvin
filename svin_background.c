#include <yaul.h>
#include <vdp1/vram.h>
#include <bcl.h>
#include <svin.h>
//#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/fs/cd/cdfs-internal.h>

vdp1_cmdt_list_t *_svin_cmdt_list;
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

void _svin_background_set(char * filename)
{
    //searching for fad
    fad_t _bg_fad;
    int iSize;
    bool b = _svin_filelist_search(filename,&_bg_fad,&iSize);
    assert(true==b);    
    _svin_background_set_by_fad(_bg_fad,iSize);
}

void _svin_background_set_no_filelist(char * filename)
{
    //searching for fad
    cdfs_filelist_t _filelist;
    cdfs_filelist_entry_t _filelist_entries[_SVIN_FILELIST_ENTRIES_PER_DIR_LIMIT];
    _filelist.entries = _filelist_entries;
    fad_t _bg_fad=0;
    int iSize=0;
	
	//--------------------------------- bad yabause check
	_filelist.entries_count = 0;
    _filelist.entries_pooled_count = 0;
    cdfs_pvd_t * pvd = malloc(sizeof(cdfs_pvd_t));
    cdfs_dirent_t *dirent_root;

    //reading pvd
    _svin_cd_block_sector_read(LBA2FAD(16), (uint8_t*)pvd);
    dirent_root = (cdfs_dirent_t *)((pvd->root_directory_record)); 
    //getting root size
    int root_length = isonum_733(dirent_root->data_length);
    root_length = (((root_length-1)/2048)+1)*2048;
    int root_start = isonum_733(dirent_root->extent);
    if (root_start <= 0)
	{
		_svin_textbox_init();
		_svin_textbox_print("","This game does not work in Yabause except romulo builds. Get one from https://github.com/razor85/yabause/releases/latest","Lato_Black15",7,7);
		while (1);
	}
	//--------------------------------- bad yabause check
	
	
    //can't use a filelist-based search here, using "normal" 8.3 search
#ifdef ROM_MODE
    cdfs_rom_filelist_root_read(&_filelist,_SVIN_FILELIST_ENTRIES_PER_DIR_LIMIT);
#else
    cdfs_filelist_entry_t * filelist_entries = cdfs_entries_alloc(-1);
    assert(filelist_entries != NULL);
    cdfs_filelist_default_init(&_filelist, filelist_entries, -1);
    cdfs_filelist_root_read(&_filelist);
#endif

    for (uint32_t i=0;i<_filelist.entries_count;i++)
    {
        if (strcmp(_filelist.entries[i].name,filename) == 0)
        {
            _bg_fad = _filelist.entries[i].starting_fad;
            iSize = _filelist.entries[i].size;
        }
    }

    assert(_bg_fad > 150);
    
    _svin_background_set_by_fad(_bg_fad,iSize);

    //we don't need yaul's filelist entries, because we have our own, freeing them
    free(filelist_entries);
}

void 
_svin_background_set_by_fad(fad_t fad, int size)
{
    //allocate memory for 77 sectors
    uint8_t *buffer = malloc(77 * 2048);
    assert((int)(buffer) > 0);
    //allocate memory for cram
    uint8_t *palette = malloc(2048);
    assert((int)(palette) > 0);

    //set zero palette to hide loading
    _svin_clear_palette(0);

    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    //reading 2nd block to check if compressed
    _svin_cd_block_sector_read(fad + 1, buffer);
    if ( (buffer[0] == 'L') && (buffer[1] == 'Z') && (buffer[2] == '7') && (buffer[3] == '7') )
    {
        //compressed, decompressing
        //getting size

        int * p32 = (int *)buffer;
        int compressed_size = p32[1];
        assert(compressed_size > 0);
        assert(compressed_size < 0x1000000);
        int compressed_size_sectors = ((compressed_size-1)/2048)+1;
        //reallocate buffer
        free(buffer);
        buffer = malloc(compressed_size_sectors*2048);

        //read compressed data
        _svin_cd_block_sectors_read(fad + 1, buffer, compressed_size+8);

        //decompress
        bcl_lz_decompress(&(buffer[8]),vdp1_vram_partitions.texture_base,compressed_size);

        //set palette
        _svin_cd_block_sector_read(fad + compressed_size_sectors + 1, palette);
        _svin_set_palette(0, palette);
    }
    else
    {
        //uncompressed, loading as usual 

        //checking if found file is the exact size we expect
        assert(size == (_svin_videomode_x_res*_svin_videomode_y_res + 2048*2));

        //reading first half of the background
        _svin_cd_block_sectors_read(fad + 1, buffer, 2048 * 77);
        scu_dma_transfer(0, (void *)(vdp1_vram_partitions.texture_base + 0 * 2048), buffer, 2048 * 77);

        //reading second half of the background
        _svin_cd_block_sectors_read(fad + 1 + 77, buffer, 2048 * 77);
        scu_dma_transfer(0, (void *)(vdp1_vram_partitions.texture_base + 77 * 2048), buffer, 2048 * 77);

        //read palette
        _svin_cd_block_sector_read(fad + 1 + 154, palette);
        _svin_set_palette(0, palette);
    }

    free(palette);
    free(buffer);
}

void _svin_background_update(char *filename)
{
    //searching for fad
    fad_t _bg_fad;
    int iSize;
    //assert(true == _svin_filelist_search(filename,&_bg_fad,&iSize));
    if (false == _svin_filelist_search(filename,&_bg_fad,&iSize))
    {
        char * pDebug = (char *)_svin_alloc_lwram(0x800,0x202FF800);
        strcpy(pDebug,filename);
        assert(0);
    }

    
    //checking if found file is the exact size we expect
    assert(iSize == (_svin_videomode_x_res*_svin_videomode_y_res + 2048*2));

    //allocate memory for 154 sectors
    uint8_t *buffer = malloc(154 * 2048);
    assert((int)(buffer) > 0);
    //allocate memory for cram
    uint8_t *palette = malloc(2048);
    assert((int)(palette) > 0);

    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    //read next image while fading-to-black current one
    for (int i = 0; i < 14; i++)
    {
        _svin_cd_block_sectors_read(_bg_fad + 1 + i * 11, &(buffer[11 * 2048 * i]), 11 * 2048);
        _svin_background_fade_to_black_step();
    }

    //continue fading-to-black
    for (int i = 0; i < 18; i++)
    {
        _svin_delay(50);
        _svin_background_fade_to_black_step();
    }

    //copy preloaded pattern names
    scu_dma_transfer(0, (void *)(vdp1_vram_partitions.texture_base), buffer, 2048 * 154);

    //read palette
    _svin_cd_block_sector_read(_bg_fad + 1 + 154, palette);
    _svin_set_palette(0, palette);

    free(buffer);
    free(palette);
}

void _svin_background_clear()
{
    //set zero palette
    _svin_clear_palette(0);
}
