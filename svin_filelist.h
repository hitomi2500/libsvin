#ifndef _SVIN_FILELIST_H_
#define _SVIN_FILELIST_H_

#define _SVIN_FILELIST_ADDRESS 0x20200000
#define _SVIN_FILELIST_ENTRIES_PER_DIR_LIMIT 256

void _svin_filelist_fill();
fad_t _svin_filelist_search(char * filename);

#endif