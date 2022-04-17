#pragma once

#include <stdint.h>

typedef enum amiss_img_fmt_e
{
    AMISS_IMG_FMT_PPM
} amiss_img_fmt_et;

typedef struct amiss_img_s
{
    uint32_t w;
    uint32_t h;
    uint32_t blen;
    uint8_t *b;
    amiss_img_fmt_et fmt;
} amiss_img_st;

uint8_t amiss_img_depth(amiss_img_st const *const img);
uint32_t amiss_img_xy2idx(amiss_img_st const *const img, uint8_t const depth,
                          uint32_t const x, uint32_t const y);
int amiss_img_save(amiss_img_st const *const img, char const *const path);
void amiss_img_flip_vert(amiss_img_st const *const img);
