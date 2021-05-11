#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)

typedef struct {
    const char *fontname;
    const char *filename;
    const char *text;
    bool justify;
    enum mf_align_t alignment;
    int width;
    int margin;
    int anchor;
    int scale;
} options_t;

typedef struct {
    options_t *options;
    uint8_t *buffer;
    uint16_t width;
    uint16_t height;
    uint16_t y;
    const struct mf_font_s *font;
} state_t;

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
bool count_lines(const char *line, uint16_t count, void *state)
{
        UNUSED(line);
        UNUSED(count);
        int *linecount = (int*)state;
        (*linecount)++;
        return true;
}

void
debug_render_text(const char * text, const char * fontname)
{
        int height;
        const struct mf_font_s *font;
        options_t options;
        state_t state = {};

        //fill background sprites with zeros
        vdp1_vram_partitions_t vdp1_vram_partitions;
        vdp1_vram_partitions_get(&vdp1_vram_partitions);

        buffer = malloc(77 * 2048);

        // Text rendering stuff

        int iStartLine = 0;
        //first fill state
        memset(&options, 0, sizeof(options_t));
        options.fontname = fontname;
        options.text = text;
        options.filename = NULL;
        options.width = 704;
        options.margin = 5;
        options.scale = 1;
        options.alignment = MF_ALIGN_LEFT;
        options.anchor = options.margin;

        font = mf_find_font(options.fontname);

        assert(font != NULL);
        
        //Count the number of lines that we need. 
        height = 0;
        mf_wordwrap(font, options.width - 2 * options.margin,
                        options.text, count_lines, &height);
        height *= font->height;
        height += 4;

        // Allocate and clear the image buffer
        state.options = &options;
        state.width = options.width;
        state.height = height;
        state.buffer = buffer;//malloc(options.width * height);
        state.y = 2;
        state.font = font;

        // Initialize image to white
        memset(state.buffer, 255, options.width * height);

        // Render the text 
        mf_wordwrap(font, options.width - 2 * options.margin,
                options.text, line_callback, &state);

        //copy buffer
        uint8_t _buf;
        uint8_t * _p;
        _p = (uint8_t *)(vdp1_vram_partitions.texture_base);
        for (int i=0;i<state.height/2; i++)
        {
                /*memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + 77 *    0 + (i+iStartLine)*352 ), &(buffer[i*1408]), 352);
                memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + 77 * 1024 + (i+iStartLine)*352 ), &(buffer[i*1408+352]), 352);
                memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + 77 * 2048 + (i+iStartLine)*352 ), &(buffer[i*1408+352*2]), 352);
                memcpy((uint8_t *)(vdp1_vram_partitions.texture_base + 77 * 3072 + (i+iStartLine)*352 ), &(buffer[i*1408+352*3]), 352);
                */
                for (int l=0;l<352;l++)
                {
                        _buf = buffer[i*1408 + l];
                        if (_buf!=0xFF) 
                                _p[77 *    0 + (i+iStartLine)*352 + l] = _buf;
                        _buf = buffer[i*1408+352+l];
                        if (_buf!=0xFF) 
                                _p[77 * 1024 + (i+iStartLine)*352 + l] = _buf;
                        _buf = buffer[i*1408+352*2+l];
                        if (_buf!=0xFF) 
                                _p[77 * 2048 + (i+iStartLine)*352 + l] = _buf;
                        _buf = buffer[i*1408+352*3+l];
                        if (_buf!=0xFF) 
                                _p[77 * 3072 + (i+iStartLine)*352 + l] = _buf;

                }
        }

        iStartLine += state.height/2;
        iStartLine += 4;
        
        free(buffer);
}