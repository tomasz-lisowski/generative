#include <stdint.h>

#pragma once

typedef enum amiss_img_fmt_e
{
    AMISS_IMG_FMT_PPM
} amiss_img_fmt_et;

typedef struct amiss_img_s
{
    uint16_t w;
    uint16_t h;
    uint32_t blen;
    uint8_t *b;
    amiss_img_fmt_et fmt;
} amiss_img_st;

int amiss_img_save(amiss_img_st *const img, char const *const path);
