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
bool line_callback(const char *line, uint16_t count, void *state)
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
