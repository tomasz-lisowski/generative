#pragma once

#include "amiss/img.h"
#include <math.h>
#include <stdbool.h>

typedef struct vec2u32_s
{
    union {
        uint32_t a[2U];
        struct
        {
            uint32_t x;
            uint32_t y;
        } __attribute__((packed));
    };
} vec2u32_st;

typedef struct color_s
{
    union {
        uint8_t a[3U];
        struct
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        } __attribute__((packed));
    };
} color_st;

typedef struct gradient_st
{
    uint8_t count;
    double_t *stops;
    color_st *colors;
} gradient_st;

uint32_t amiss_draw_line(amiss_img_st const *const img, color_st const color,
                         double_t const thickness, bool const antialias,
                         vec2u32_st const start, vec2u32_st const end);

void amiss_draw_bg_gradient(amiss_img_st const *const img,
                            gradient_st const gradient);
