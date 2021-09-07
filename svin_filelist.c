#include <yaul.h>
#include <stdlib.h>
#include "svin_filelist.h"
#include "cd-block_multiread.h"
#include "iso9660-internal-local.h"

int _svin_filelist_size;
char *_svin_filelist_long_filename;

bool
_svin_filelist_search_lfn(iso9660_filelist_entry_t entry, char * raw_buffer, int parent_raw_buffer_len, bool bFolder)
{
    char search_buf[16];
    strcpy(search_buf,entry.name);
    assert(strlen(search_buf)<=12);
    if (bFolder)
        strcat(search_buf,";1");
    int len = strlen(search_buf);
    //bool bFound = false;

    //for (int i=0;(i<8100)&&(bFound==false);i++)
    for (int i=0;i<parent_raw_buffer_len-len-1;i++)
    {
        if (0==memcmp(search_buf,raw_buffer+i,len))
        {
            //bFound = true;
            //scanning forward until NM signature
            while ((i<8100)&&(0!=memcmp("NM",raw_buffer+i,2)))
                i++;
            len = raw_buffer[i+2]-5;
            memcpy(_svin_filelist_long_filename,(uint8_t*)&(raw_buffer[i+5]),len);
            _svin_filelist_long_filename[len] = 0;
            //strcpy(_svin_filelist_long_filename,"Some long filename");
            return true;
        }
    }

    return false;
}

void 
_svin_filelist_fill_recursive(iso9660_filelist_entry_t entry, char * current_dir, char * parent_raw_buffer, int parent_raw_buffer_len)
{
    iso9660_filelist_t _rec_filelist;
    iso9660_filelist_entry_t _rec_filelist_entries[_SVIN_FILELIST_ENTRIES_PER_DIR_LIMIT];
    _rec_filelist.entries = _rec_filelist_entries;
    _rec_filelist.entries_count = 0;
    _rec_filelist.entries_pooled_count = 0;
    uint32_t _rec_i;
    char dir[260]; //remember that MAX_PATH horror?
    char * raw_buffer;
    int raw_size;

    fad_t * _rec_pfad = (fad_t*)(_SVIN_FILELIST_ADDRESS+256*_svin_filelist_size);
    char * _rec_p8 = (char*)(_SVIN_FILELIST_ADDRESS+256*_svin_filelist_size+4);
    
    if (ISO9660_ENTRY_TYPE_FILE == entry.type) {
        _rec_pfad[0] = entry.starting_fad;
        if (strlen(current_dir)>0)
        {
            strcpy(_rec_p8,current_dir);
            strcat(_rec_p8,"/");
        }
        else
            strcpy(_rec_p8,"");
        //we need to figure out long filename
        //using dir buffer for thar purpose
        if (_svin_filelist_search_lfn(entry,parent_raw_buffer,parent_raw_buffer_len,true))
        {
            strcat(_rec_p8,_svin_filelist_long_filename);
        }
        else
        {
            strcat(_rec_p8,entry.name); //using short filename if no LFN is available
        }    
        _svin_filelist_size++;
    }
    else if (ISO9660_ENTRY_TYPE_DIRECTORY == entry.type){
        //recursively allocating another buffer
        raw_size = 8192;//TODO: detect this automatically
        raw_buffer = malloc(raw_size);
        //now read raw folder content for LFN search
        cd_block_multiple_sectors_read(entry.starting_fad, 4, (uint8_t*)raw_buffer);

        strcpy(dir,current_dir);
        strcat(dir,"/");
        if (_svin_filelist_search_lfn(entry,parent_raw_buffer,parent_raw_buffer_len,false))
        {
            strcat(dir,_svin_filelist_long_filename);
        }
        else
        {
            strcat(dir,entry.name); //using short filename if no LFN is available
        }    
        iso9660_filelist_read(entry,&_rec_filelist,_SVIN_FILELIST_ENTRIES_PER_DIR_LIMIT); 

        for (_rec_i=0;_rec_i<_rec_filelist.entries_count;_rec_i++)
        {
            _svin_filelist_fill_recursive(_rec_filelist.entries[_rec_i],dir,raw_buffer,raw_size);
        }
        free(raw_buffer);
    }
}

//load entire files list into LWRAM
void 
_svin_filelist_fill()
{
    iso9660_filelist_t _filelist;
    iso9660_filelist_entry_t _filelist_entries[_SVIN_FILELIST_ENTRIES_PER_DIR_LIMIT];
    _filelist.entries = _filelist_entries;
    _filelist.entries_count = 0;
    _filelist.entries_pooled_count = 0;
    uint32_t i;
    iso9660_pvd_t * pvd = malloc(sizeof(iso9660_pvd_t));
    iso9660_dirent_t dirent_root;

    _svin_filelist_size = 0;

    //reading pvd
    cd_block_sector_read(LBA2FAD(16), (uint8_t*)pvd);
            
    //getting root size
    memcpy(&dirent_root, pvd->root_directory_record, sizeof(dirent_root));
    int root_length = isonum_733(dirent_root.data_length);
    root_length = (((root_length-1)/2048)+1)*2048;
    int root_start = isonum_733(dirent_root.extent);
    char * raw_buffer = malloc(root_length);
    cd_block_multiple_sectors_read(LBA2FAD(root_start), root_length/2048, (uint8_t*)raw_buffer);

    //allocating global  buffer for filename
    _svin_filelist_long_filename = malloc(256);//ought to be enough for everyone

    iso9660_filelist_root_read(&_filelist, _SVIN_FILELIST_ENTRIES_PER_DIR_LIMIT);

    for (i=0;i<_filelist.entries_count;i++)
        _svin_filelist_fill_recursive(_filelist.entries[i],"",raw_buffer,root_length);

    free (raw_buffer);
    free (_svin_filelist_long_filename);
    free (pvd);
}

//lsearches the filelist and returns fad (-1 if not found)
fad_t 
_svin_filelist_search(char * filename)
{
    fad_t * pfad = (fad_t *)(_SVIN_FILELIST_ADDRESS);
    //doing a simple linear search for now
    for (int i=0;i<_svin_filelist_size;i++)
    {
        if (strcmp(filename,(char*)(_SVIN_FILELIST_ADDRESS+i*256+4)) == 0)
        {
            return pfad[i*64];
        }
    }
    return -1;
}