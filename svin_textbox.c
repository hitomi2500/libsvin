#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"
#include "svin_text.h"
#include "svin_textbox.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)

uint8_t *buffer;

void 
_svin_textbox_init()
{
    int *_pointer32;

    _svin_set_cycle_patterns_cpu();

    //-------------- setup pattern names -------------------
    _pointer32 = (int *)_SVIN_NBG2_PNDR_START;
    //writing semi-transparent characters where the dialog box should go, 
    int index = 0;
    int iOffset;
    for (int y = 44; y < 54; y++)
    {
        //plane 0 first
        iOffset = y * 64;
        for (int x = 4; x < 64; x++)
        {
            _pointer32[iOffset + x] = 0x10700000 + _SVIN_NBG2_CHPNDR_TEXTBOX_INDEX + _SVIN_CHARACTER_UNITS * index; //palette 7, transparency on
            index++;
        }
        //now plane 1
        iOffset = 64 * 64 + y * 64;
        for (int x = 0; x < 20; x++)
        {
            _pointer32[iOffset + x] = 0x10700000 + _SVIN_NBG2_CHPNDR_TEXTBOX_INDEX + _SVIN_CHARACTER_UNITS * index; //palette 7, transparency on
            index++;
        }
    }

    _svin_set_cycle_patterns_nbg();

    _svin_textbox_clear();

}

void 
_svin_textbox_clear()
{
    //-------------- setup character pattern names ------------------
    //setting up textbox placeholder for nbg2, 5x40 chars = 640x80

    _svin_set_cycle_patterns_cpu();

    int * _pointer32 = (int *)(_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR);
    for (unsigned int i = 0; i < (_SVIN_NBG2_CHPNDR_TEXTBOX_SIZE) / sizeof(int); i+=4)
    {
        _pointer32[i] = 0x0F000F00;
        _pointer32[i+1] = 0x0F000F00;
        _pointer32[i+2] = 0x000F000F;
        _pointer32[i+3] = 0x000F000F;
    }

    _svin_set_cycle_patterns_nbg();
}

void 
_svin_textbox_disable()
{
    _svin_set_cycle_patterns_cpu();
    //filling entire textbox range with transparent color 0
    memset((void*)_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR,0,_SVIN_NBG2_CHPNDR_TEXTBOX_SIZE);
    _svin_set_cycle_patterns_nbg();
}

void
_svin_textbox_print(const char * speaker, const char * text, const char * fontname, int speaker_color, int text_color)
{
        int height=0;
        uint8_t _buf;
        uint8_t * _p;

        buffer = malloc(32 * 2048);

        _svin_textbox_clear();

        // Rendering speaker name first, 1st line, shifted 1 quad to the right

        _svin_set_cycle_patterns_cpu();

        if (strlen(speaker)>0)
        {
                // Render the speaker name 
                height = _svin_text_render(buffer,640,speaker,fontname);

                //assert (height <= 16);

                //copy speaker name
                _p = (uint8_t *)(_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR);
                for (int cellX = 0; cellX < 80; cellX++)
                {
                        for (int cellY = 0; cellY < (height/8); cellY++)
                        {
                                for (int x=0;x<8;x++)
                                {
                                        for (int y=0;y<8;y++)
                                        {
                                                _buf = buffer[((cellY)*8+y+4)*640+cellX*8+x];
                                                if (_buf!=0xFF) 
                                                {
                                                        _buf = speaker_color*16 + _buf/16;
                                                        if (0==_buf)
                                                                _buf = 1;//0 is a transparency color, using close value
                                                        _p[(cellY * 80 + cellX+1)*_SVIN_CHARACTER_BYTES + y*8 + x] = _buf;
                                                }

                                        }
                                }
                        }
                }
        }

        //char eee[256];
        //sprintf(eee,"%i",height);
        // Now rendering the actual text from cellY = 1
        height = _svin_text_render(buffer,640,text,fontname);
        //height = _svin_text_render(buffer,640,eee,fontname);

        //assert (height <= 64);

        _p = (uint8_t *)(_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR);
        for (int cellX = 0; cellX < 80; cellX++)
        {
                for (int cellY = 2; cellY < 2 + (height/8); cellY++)
                {
                        for (int x=0;x<8;x++)
                        {
                                for (int y=0;y<8;y++)
                                {
                                        //_buf = buffer[((cellY-1)*8+y)*640+cellX*8+x];
                                        _buf = buffer[((cellY-2)*8+y)*640+cellX*8+x];
                                        if (_buf!=0xFF) 
                                        {
                                                _buf = text_color*16 + _buf/16;
                                                if (0==_buf)
                                                        _buf = 1;//0 is a reserved color, using close value
                                                _p[(cellY*80+cellX)*_SVIN_CHARACTER_BYTES + y*8 + x] = _buf;
                                        }

                                }
                        }
                }
        }

        _svin_set_cycle_patterns_nbg();

        free(buffer);
}

