#include "amiss.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMG_SIZE 1080

int main()
{
    uint32_t const buf_size = IMG_SIZE * IMG_SIZE * 3U /* RGB */;
    uint8_t *buf = malloc(buf_size);
    if (buf == NULL)
    {
        return 1;
    }
    amiss_img_st img = {
        .b = buf,
        .blen = buf_size,
        .fmt = AMISS_IMG_FMT_PPM,
        .h = IMG_SIZE,
        .w = IMG_SIZE,
    };

    color_st color_bg = {.r = 27, .g = 11, .b = 9};
    for (uint32_t y = 0U; y < img.h; ++y)
    {
        for (uint32_t x = 0U; x < img.w; ++x)
        {
            amiss_draw_px_set(&img, color_bg, x, y);
        }
    }

    /* Init seed for RNG. */
    srand(0x6C6F7665);

    uint16_t const grid_size = 20U;
    color_st const color_stitch = {.r = 180, .g = 90, .b = 80};
    uint32_t const margin_size = grid_size * 2U;

    /* Vertical stitches. */
    for (uint32_t y = margin_size; y < img.h - (margin_size / 2U);
         y += grid_size)
    {
        vec2u32_st start = {.x = (uint32_t)rand() > (RAND_MAX / 2U)
                                     ? (margin_size / 2U) + (grid_size * 2U)
                                     : (margin_size / 2U) + grid_size,
                            .y = y};
        while (start.x < img.w - margin_size)
        {
            vec2u32_st end = {.x = start.x + grid_size, .y = start.y};
            amiss_draw_line(&img, color_stitch, 1.0, false, start, end);
            amiss_draw_line(&img, color_stitch, 1.0, false,
                            (vec2u32_st){.x = start.x, .y = start.y + 1U},
                            (vec2u32_st){.x = end.x, .y = end.y + 1U});
            start.x += grid_size * 2U;
        }
    }

    for (uint32_t x = margin_size; x < img.w - (margin_size / 2U);
         x += grid_size)
    {
        vec2u32_st start = {.x = x,
                            .y = (uint32_t)rand() > (RAND_MAX / 2U)
                                     ? (margin_size / 2U) + (grid_size * 2U)
                                     : (margin_size / 2U) + grid_size};
        while (start.y < img.h - margin_size)
        {
            vec2u32_st end = {.x = start.x, .y = start.y + grid_size};
            amiss_draw_line(&img, color_stitch, 1.0, false, start, end);
            amiss_draw_line(&img, color_stitch, 1.0, false,
                            (vec2u32_st){.x = start.x + 1U, .y = start.y},
                            (vec2u32_st){.x = end.x + 1U, .y = end.y + 1U});
            start.y += grid_size * 2U;
        }
    }

    return amiss_img_save(&img, "002-hitomezashi.ppm");
}
