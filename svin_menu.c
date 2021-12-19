#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"
#include "svin_textbox.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)



void 
_svin_menu_init()
{
    int *_pointer32;

    _svin_set_cycle_patterns_cpu();

    //-------------- setup pattern names -------------------
    _pointer32 = (int *)_SVIN_NBG2_PNDR_START;

    //clearing textbox
    for (int y = 44; y < 54; y++)
    {
        //plane 0 first
        iOffset = y * 64;
        for (int x = 4; x < 64; x++)
        {
            _pointer32[iOffset + x] = 0x00000000 + _SVIN_NBG2_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
        }
        //now plane 1
        iOffset = 64 * 64 + y * 64;
        for (int x = 0; x < 20; x++)
        {
            _pointer32[iOffset + x] = 0x00000000 + _SVIN_NBG2_CHPNDR_SPECIALS_INDEX; //palette 0, transparency on
            index++;
        }
    }

    //menu is 40x20 tiles, to fit into a single plane, and into the same tilespace as dialog 
    int index = 0;
    int iOffset;
    for (int y = 25; y < 45; y++)
    {
        //plane 0 only
        iOffset = y * 64;
        for (int x = 24; x < 64; x++)
        {
            _pointer32[iOffset + x] = 0x10700000 + _SVIN_NBG2_CHPNDR_TEXTBOX_INDEX + _SVIN_CHARACTER_UNITS * index; //palette 7, transparency on
            index++;
        }
    }

    _svin_set_cycle_patterns_nbg();
}

void 
_svin_menu_populate(int index, const char * text, const char * fontname)
{
        
}


void 
_svin_menu_clear()
{
    //-------------- setup character pattern names ------------------
    //setting up textbox placeholder for nbg2, 5x40 chars = 640x80

    int * _pointer32 = (int *)(_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR);
    for (unsigned int i = 0; i < (_SVIN_NBG2_CHPNDR_TEXTBOX_SIZE) / sizeof(int); i+=4)
    {
        _pointer32[i] = 0x0F000F00;
        _pointer32[i+1] = 0x0F000F00;
        _pointer32[i+2] = 0x000F000F;
        _pointer32[i+3] = 0x000F000F;
    }
}

void 
_svin_menu_disable()
{
    //filling entire textbox range with transparent color 0
    memset((void*)_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR,0,_SVIN_NBG2_CHPNDR_TEXTBOX_SIZE);
}

void
_svin_menu_print(const char * speaker, const char * text, const char * fontname, int speaker_color, int text_color)
{
        int height;
        const struct mf_font_s *font;
        options_t options;
        state_t state = {};
        uint8_t _buf;
        uint8_t * _p;

        //fill background sprites with zeros
        vdp1_vram_partitions_t vdp1_vram_partitions;
        vdp1_vram_partitions_get(&vdp1_vram_partitions);

        buffer = malloc(32 * 2048);

        _svin_textbox_clear();

        // Rendering speaker name first, 1st line, shifted 1 quad to the right

        //first fill state
        memset(&options, 0, sizeof(options_t));
        options.fontname = "Lato_Black12";
        options.text = speaker;
        options.filename = NULL;
        options.width = 640;
        options.margin = 5;
        options.scale = 1;
        options.alignment = MF_ALIGN_LEFT;
        options.anchor = options.margin;

        font = mf_find_font(options.fontname);

        assert(font != NULL);
        
        //Count the number of lines that we need. 
        height = 0;
        mf_wordwrap(font, options.width - 2 * options.margin, options.text, count_lines, &height);
        height *= font->height;
        height += 4;

        // Setup the image buffer
        state.options = &options;
        state.width = options.width;
        state.height = height;
        state.buffer = buffer;
        state.y = 2;
        state.font = font;

        // Initialize image to white
        memset(state.buffer, 255, options.width * height);

        if ((height > 0)&&(strlen(speaker)>0))
        {
                // Render the speaker name 
                mf_wordwrap(font, options.width - 2 * options.margin, options.text, line_callback, &state);

                //copy speaker name
                _p = (uint8_t *)(_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR);
                for (int cellX = 0; cellX < 80; cellX++)
                {
                        for (int cellY = 0; cellY < 2; cellY++)
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


        // Now rendering the actual text from cellY = 1

        //first fill state
        memset(&options, 0, sizeof(options_t));
        options.fontname = fontname;
        options.text = text;
        options.filename = NULL;
        options.width = 640;
        options.margin = 5;
        options.scale = 1;
        options.alignment = MF_ALIGN_LEFT;
        options.anchor = options.margin;

        font = mf_find_font(options.fontname);

        assert(font != NULL);
        
        //Count the number of lines that we need. 
        height = 0;
        mf_wordwrap(font, options.width - 2 * options.margin, options.text, count_lines, &height);
        height *= font->height;
        height += 4;

        // Setup the image buffer
        state.options = &options;
        state.width = options.width;
        state.height = height;
        state.buffer = buffer;
        state.y = 2;
        state.font = font;

        // Initialize image to white
        memset(state.buffer, 255, options.width * 80);//height); 

        // Render the text
        mf_wordwrap(font, options.width - 2 * options.margin, options.text, line_callback, &state);

        _p = (uint8_t *)(_SVIN_NBG2_CHPNDR_TEXTBOX_ADDR);
        for (int cellX = 0; cellX < 80; cellX++)
        {
                for (int cellY = 2; cellY < 10; cellY++)
                {
                        for (int x=0;x<8;x++)
                        {
                                for (int y=0;y<8;y++)
                                {
                                        _buf = buffer[((cellY-1)*8+y)*640+cellX*8+x];
                                        if (_buf!=0xFF) 
                                        {
                                                _buf = text_color*16 + _buf/16;
                                                if (0==_buf)
                                                        _buf = 1;//0 is a reserved coloer, using close value
                                                _p[(cellY*80+cellX)*_SVIN_CHARACTER_BYTES + y*8 + x] = _buf;
                                        }

                                }
                        }
                }

        }

        free(buffer);
}

