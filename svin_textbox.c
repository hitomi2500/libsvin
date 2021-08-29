#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"
#include "svin_textbox.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)

uint8_t *buffer;
//static const char default_text[] = "The quick brown fox jumps over the lazy dog. ";

/* Callback to write to a memory buffer. */
static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha,
                           void *state)
{
    state_t *s = (state_t*)state;
    uint32_t pos;
    int16_t value;
    
    if (y < 0 || y >= s->height) return;
    if (x < 0 || x + count >= s->width) return;
    
    while (count--)
    {
        pos = (uint32_t)s->width * y + x;
        value = s->buffer[pos];
        value -= alpha;
        if (value < 0) value = 0;
        s->buffer[pos] = value;
        
        x++;
    }
}

/* Callback to render characters. */
static uint8_t character_callback(int16_t x, int16_t y, mf_char character,
                                  void *state)
{
    state_t *s = (state_t*)state;
    return mf_render_character(s->font, x, y, character, pixel_callback, state);
}

/* Callback to render lines. */
static bool line_callback(const char *line, uint16_t count, void *state)
{
    state_t *s = (state_t*)state;
    
    if (s->options->justify)
    {
        mf_render_justified(s->font, s->options->anchor, s->y,
                            s->width - s->options->margin * 2,
                            line, count, character_callback, state);
    }
    else
    {
        mf_render_aligned(s->font, s->options->anchor, s->y,
                          s->options->alignment, line, count,
                          character_callback, state);
    }
    s->y += s->font->line_height;
    return true;
}

/* Callback to just count the lines.
 * Used to decide the image height */
static bool count_lines(const char *line, uint16_t count, void *state)
{
        UNUSED(line);
        UNUSED(count);
        int *linecount = (int*)state;
        (*linecount)++;
        return true;
}


void 
_svin_textbox_init()
{
    int *_pointer32;

    //-------------- setup pattern names -------------------
    _pointer32 = (int *)_SVIN_NBG1_PNDR_START;
    //writing semi-transparent characters where the dialog box should go, 
    int index = 0;
    int iOffset;
    for (int y = 22; y < 27; y++)
    {
        //plane 0 first
        iOffset = y * 32;
        for (int x = 2; x < 32; x++)
        {
            _pointer32[iOffset + x] = 0x10700000 + _SVIN_NBG1_CHPNDR_TEXTBOX_INDEX + _SVIN_CHARACTER_UNITS * index; //palette 7, transparency on
            //_pointer32[iOffset + x] = 0x00000000 + (0x100000*((x-2)%8)) + _SVIN_NBG1_CHPNDR_TEXTBOX_INDEX + _SVIN_CHARACTER_UNITS * index; //palette 1, transparency off
            index++;
        }
        //now plane 1
        iOffset = 32 * 32 + y * 32;
        for (int x = 0; x < 10; x++)
        {
            _pointer32[iOffset + x] = 0x10700000 + _SVIN_NBG1_CHPNDR_TEXTBOX_INDEX + _SVIN_CHARACTER_UNITS * index; //palette 7, transparency on
            //_pointer32[iOffset + x] = 0x00000000 + 0x100000*(y-22) + _SVIN_NBG1_CHPNDR_TEXTBOX_INDEX + _SVIN_CHARACTER_UNITS * index; //palette 1, transparency off
            index++;
        }
    }

    //-------------- setup character pattern names ------------------

    //setting up textbox placeholder for nbg1, 5x40 chars = 640x80
    _pointer32 = (int *)(_SVIN_NBG1_CHPNDR_TEXTBOX_ADDR);
    for (unsigned int i = 0; i < (_SVIN_CHARACTER_BYTES*200) / sizeof(int); i++)
    {
        _pointer32[i] = 0x0F0F0F0F;//0x7F7F7F7F;
    }

    //-------------- setup palette 7 specifically for text  -------------------

    _svin_textbox_init_palette(); //disable textbox by default, will be enabled when required

}

void 
_svin_textbox_init_palette()
{
    //filling pallete with valid colors to enable text layer
   //setup default palettes
    uint8_t temp_pal[3 * 256];

    //pallete 7 is special. it's 16 gradients from solid colors to "backgroung frame" color  
    //reverse grayscale
    int iBaseR,iBaseG,iBaseB;
    int iStepR,iStepG,iStepB;
    for (int iColor=0; iColor<16; iColor++)
    {
        iBaseR = 0x7F; iBaseG = 0x7F; iBaseB = 0x7F; 
        switch(iColor)
        {
            case 0: //quasi-black
                iStepR = -8; iStepG = -8; iStepB = -8;
                break;
            case 1: //red
                iStepR = 8; iStepG = -7; iStepB = -7;
                break;
            case 2: //green
                iStepR = -7; iStepG = 8; iStepB = -7;
                break;
            case 3: //blue
                iStepR = -7; iStepG = -7; iStepB = 8;
                break;
            case 4: //cyan
                iStepR =-7; iStepG = 8; iStepB = 8;
                break;
            case 5: //magenta
                iStepR = 8; iStepG = -7; iStepB = 8;
                break;
            case 6: //yellow
                iStepR = 8; iStepG = 8; iStepB = -7;
                break;
            case 7: //white
                iStepR = 8; iStepG = 8; iStepB = 8;
                break;
            case 8: //orange
                iStepR = 6; iStepG = 0; iStepB = -6;
                break;
            case 9: //olive
                iStepR = 0; iStepG = 4; iStepB = -4;
                break;
            case 10: //brown
                iStepR = 2; iStepG = 0; iStepB = -7;
                break;
            case 11: //lightblue
                iStepR = 0; iStepG = 4; iStepB = 8;
                break;
            case 12: //meaty
                iStepR = 6; iStepG = 0; iStepB = 0;
                break;
            case 13: //khaki
                iStepR = -7; iStepG = -2; iStepB = 0;
                break;
            case 14: //grey
                iStepR = -2; iStepG = -2; iStepB = -2;
                break;
            case 15: //coffee
                iStepR = 6; iStepG = 4; iStepB = -2;
                break;
        }

        for (int i = 15; i >= 0; i--)
        {
            temp_pal[iColor*48 + i * 3] = iBaseR;     
            temp_pal[iColor*48 + i * 3 + 1] = iBaseG; 
            temp_pal[iColor*48 + i * 3 + 2] = iBaseB; 
            iBaseR += iStepR;
            iBaseG += iStepG;
            iBaseB += iStepB;
        }
    }

    _svin_background_set_palette(7, temp_pal);

    _svin_textbox_clear();
}

void 
_svin_textbox_clear()
{
    //filling entire textbox range with transparent color 0
    memset((void*)_SVIN_NBG1_CHPNDR_TEXTBOX_ADDR,0,640*80);
}

void
_svin_textbox_print(const char * speaker, const char * text, const char * fontname, int speaker_color, int text_color)
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

        memset((void*)_SVIN_NBG1_CHPNDR_TEXTBOX_ADDR,0x0F,640*80);

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
                _p = (uint8_t *)(_SVIN_NBG1_CHPNDR_TEXTBOX_ADDR);
                for (int cellX = 0; cellX < 20; cellX++)
                {
                        //cellY = 0;
                        {
                                for (int cell=0; cell<4; cell++)
                                {
                                        for (int x=0;x<8;x++)
                                        {
                                                for (int y=0;y<8;y++)
                                                {
                                                        _buf = buffer[((cell/2)*8+y+4)*640+cellX*16+(cell%2)*8+x];
                                                        if (_buf!=0xFF) 
                                                        {
                                                                _buf = speaker_color*16 + _buf/16;
                                                                if (0==_buf)
                                                                        _buf = 1;//0 is a transparency color, using close value
                                                                _p[(cellX+1)*_SVIN_CHARACTER_BYTES + cell*64 + y*8 + x] = _buf;
                                                        }

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

        _p = (uint8_t *)(_SVIN_NBG1_CHPNDR_TEXTBOX_ADDR);
        for (int cellX = 0; cellX < 40; cellX++)
        {
                for (int cellY = 1; cellY < 5; cellY++)
                {
                        for (int cell=0; cell<4; cell++)
                        {
                                for (int x=0;x<8;x++)
                                {
                                        for (int y=0;y<8;y++)
                                        {
                                                _buf = buffer[((cellY-1)*16+(cell/2)*8+y)*640+cellX*16+(cell%2)*8+x];
                                                if (_buf!=0xFF) 
                                                {
                                                        _buf = text_color*16 + _buf/16;
                                                        if (0==_buf)
                                                                _buf = 1;//0 is a reserved coloer, using close value
                                                        _p[(cellY*40+cellX)*_SVIN_CHARACTER_BYTES + cell*64 + y*8 + x] = _buf;
                                                }

                                        }
                                }
                        }
                }

        }

        free(buffer);
}

