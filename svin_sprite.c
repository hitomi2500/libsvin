#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)

void 
_svin_sprite_draw(char * filename)
{
    char tmp_buffer[2048];
    bool bFinished = false;
    char * pDebug = (char*)0x20200000;
    int i,j,k;
    int iActor;
    int iActorColor;

    //first let's find sprite FAD, for that we need to go deep into file structure
    int _sprite_fad = _svin_filelist_search(filename):
    assert(_sprite_fad > 0);
 
    }   
}