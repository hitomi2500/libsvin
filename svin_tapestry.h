#ifndef _SVIN_TAPESTRY_H_
#define _SVIN_TAPESTRY_H_

//#define PORT_USE_FILECLIENT

#include <yaul.h>
#include <svin.h>

void _svin_tapestry_init();
void _svin_tapestry_load_position(iso9660_filelist_t *_filelist, char *filename, int position);
void _svin_tapestry_move_up();
void _svin_tapestry_move_down();

#endif
