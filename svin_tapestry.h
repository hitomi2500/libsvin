#ifndef _SVIN_TAPESTRY_H_
#define _SVIN_TAPESTRY_H_

#include <yaul.h>
#include <svin.h>

void _svin_tapestry_init();
void _svin_tapestry_load_position(char *filename, int position);
int _svin_tapestry_get_vsize(char *filename);
void _svin_tapestry_move_up();
void _svin_tapestry_move_down();

#endif
