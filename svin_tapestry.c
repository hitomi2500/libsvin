#include <yaul.h>
#include <vdp1/vram.h>
#include <tga.h>
#include <svin.h>
//#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

vdp1_cmdt_list_t *_svin_tapestry_cmdt_list_1;
vdp1_cmdt_list_t *_svin_tapestry_cmdt_list_2;
fad_t _svin_tapestry_pack_fad;
int _svin_tapestry_framebuffer_start;
int _svin_tapestry_data_start;
int _svin_tapestry_movement_active;

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
    vdp1_cmdt_vtx_local_coord_set(&cmdt_buf, local_coord_ul);
    vdp1_cmdt_jump_assign(&cmdt_buf,16);
    vdp1_sync_cmdt_put(&cmdt_buf,1,_SVIN_VDP1_ORDER_LOCAL_COORDS_A_INDEX);
    vdp1_sync();

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

    vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_1, 16);
    vdp1_sync();
	
    //now frame B
    (void)memset(&cmdt_buf, 0x00, sizeof(vdp1_cmdt_t));
    vdp1_cmdt_local_coord_set(&cmdt_buf);
    vdp1_cmdt_vtx_local_coord_set(&cmdt_buf, local_coord_ul);
    vdp1_cmdt_jump_assign(&cmdt_buf,480);
    vdp1_sync_cmdt_put(&cmdt_buf,1,_SVIN_VDP1_ORDER_LOCAL_COORDS_B_INDEX);
    vdp1_sync();

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

    vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_2, 480); 
    vdp1_sync();  
}

void 
_svin_tapestry_load_position(char *filename, int position)
{
    _svin_tapestry_pack_fad = 0;
    _svin_tapestry_framebuffer_start = 0;
    _svin_tapestry_data_start = 0;

    vdp1_vram_partitions_t vdp1_vram_partitions;
    vdp1_vram_partitions_get(&vdp1_vram_partitions);

    uint8_t tmp_sector[2048];

    //searching for fad
    int iSize;
    bool b = _svin_filelist_search(filename,&_svin_tapestry_pack_fad,&iSize);
    assert(true == b);
    assert(_svin_tapestry_pack_fad > 0);

    //now that we know the FAD, load 224 blocks from the position start
    for (int i=0;i<224;i++)
    {
        _svin_cd_block_sector_read(_svin_tapestry_pack_fad + 1 + position + i, tmp_sector);
        memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + i*1408), tmp_sector, 1408);
    }
	
	//now load palette, to get its position we load first blocks
	_svin_cd_block_sector_read(_svin_tapestry_pack_fad, tmp_sector);
	uint16_t * p16 = (uint16_t*) tmp_sector;
	int palette_fad_offset = p16[35] / 2;
	//getting palette itself
	_svin_cd_block_sector_read(_svin_tapestry_pack_fad + palette_fad_offset + 1, tmp_sector);
	_svin_set_palette(0,tmp_sector);
}

int 
_svin_tapestry_get_vsize(char *filename)
{
    fad_t _fad;
    uint8_t tmp_sector[2048];

    //searching for fad
    int iSize;
    bool b = _svin_filelist_search(filename,&_fad,&iSize);
    assert(true == b);
    assert(_fad > 0);

	//now load first block
	_svin_cd_block_sector_read(_fad, tmp_sector);
	uint16_t * p16 = (uint16_t*) tmp_sector;
    return p16[35];
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

        //load the sector
        /*if ( 1 == _svin_tapestry_movement_active)
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
        }*/
		//using current yaul sync api instead
		_svin_cd_block_sector_read(_svin_tapestry_pack_fad + 1 + _svin_tapestry_data_start + 0, tmp_sector);
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
        vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_1, 16); 
        vdp1_sync(); 
        vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_2, 480); 
        vdp1_sync();  
        _svin_tapestry_framebuffer_start--;
        _svin_tapestry_data_start--;
        if (_svin_tapestry_framebuffer_start < 0)
            _svin_tapestry_framebuffer_start += 250;
        //prefetching data for consecutive movement
        //_svin_cd_block_sector_read_request(_svin_tapestry_pack_fad + 1 + _svin_tapestry_data_start + 0); 
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
        //load the sector
        /*if ( -1 == _svin_tapestry_movement_active)
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
        }*/
		//using current yaul sync api instead
		_svin_cd_block_sector_read(_svin_tapestry_pack_fad + 1 + _svin_tapestry_data_start + 224, tmp_sector);
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
        vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_1, 16); 
        vdp1_sync(); 
        vdp1_sync_cmdt_list_put(_svin_tapestry_cmdt_list_2, 480); 
        vdp1_sync();  
        _svin_tapestry_framebuffer_start++;
        _svin_tapestry_data_start++;
        if (_svin_tapestry_framebuffer_start >= 250)
            _svin_tapestry_framebuffer_start = 0;
        //prefetching data for consecutive movement
        //_svin_cd_block_sector_read_request(_svin_tapestry_pack_fad + 1 + _svin_tapestry_data_start + 224); 
        _svin_tapestry_movement_active = -1;
    }
}