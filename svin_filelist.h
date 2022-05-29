#ifndef _SVIN_FILELIST_H_
#define _SVIN_FILELIST_H_

#define _SVIN_FILELIST_ENTRIES_PER_DIR_LIMIT 4096

bool _svin_filelist_fill();
bool _svin_filelist_search(char * filename, fad_t * fad, int * size);

#endif