#include "amiss.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t amiss_img_depth(amiss_img_st const *const img)
{
    return img->fmt == AMISS_IMG_FMT_PPM ? 3 : 1;
}

uint32_t amiss_img_xy2idx(amiss_img_st const *const img, uint8_t const depth,
                          uint32_t const x, uint32_t const y)
{
    return ((img->w * depth) * y) + (x * depth);
}

int amiss_img_save(amiss_img_st const *const img, char const *const path)
{
    int32_t ret = 0;
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        log_err("AMISS_IMG", "Failed to create/open file: %s\n", strerror(ret));
        return -1;
    }

    switch (img->fmt)
    {
    case AMISS_IMG_FMT_PPM:
        ret = fprintf(f, "P6\n%u %u\n255\n", img->w, img->h);
        uint32_t const img_size = img->w * img->h;
        if (img->blen < img_size * 3 /* RGB */)
        {
            log_err("AMISS_IMG",
                    "Image buffer is too small to contain the image\n");
            return -1;
        }
        uint64_t const pix_written =
            fwrite(img->b, sizeof(img->b[0]) * 3 /* RGB */, img_size, f);
        if (pix_written != img_size)
        {
            log_err("AMISS_IMG", "Failed to write pixels\n");
            ret = fclose(f);
            if (ret < 0)
            {
                log_warn("AMISS_IMG", "Failed to close file: %s\n",
                         strerror(ret));
            }
            return -1;
        }
        break;
    }

    ret = fclose(f);
    if (ret < 0)
    {
        log_warn("AMISS_IMG", "Failed to close file: %s\n", strerror(ret));
    }
    return 0;
}

void amiss_img_flip_vert(amiss_img_st const *const img)
{
    uint8_t const depth = amiss_img_depth(img);
    uint32_t const line_size = img->w * depth;
    uint8_t *line_tmp = malloc(line_size);
    if (line_tmp == NULL)
    {
        log_err("AMISS_IMG", "Failed to flip image\n");
        free(line_tmp);
        return;
    }

    for (uint32_t y = 0; y < (img->h / 2); ++y)
    {
        memcpy(line_tmp, &img->b[amiss_img_xy2idx(img, depth, 0, y)],
               line_size);
        memcpy(&img->b[amiss_img_xy2idx(img, depth, 0, y)],
               &img->b[amiss_img_xy2idx(img, depth, 0, img->h - y - 1)],
               line_size);
        memcpy(&img->b[amiss_img_xy2idx(img, depth, 0, img->h - y - 1)],
               line_tmp, line_size);
    }
    free(line_tmp);
}
