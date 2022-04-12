#include "amiss.h"
#include <stdio.h>
#include <string.h>

int amiss_img_save(amiss_img_st *const img, char const *const path)
{
    int32_t ret = 0;
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        printf("Failed to create/open file: %s\n", strerror(ret));
        return -1;
    }

    switch (img->fmt)
    {
    case AMISS_IMG_FMT_PPM:
        ret = fprintf(f, "P6\n%u %u\n255\n", img->w, img->h);
        uint32_t const img_size =
            (uint32_t)(img->w * img->h); /* NOTE: Safe cast. */
        if (img->blen < img_size * 3 /* RGB */)
        {
            printf("Image buffer is too small to contain the image\n");
            return -1;
        }
        uint64_t const pix_written =
            fwrite(img->b, sizeof(img->b[0]) * 3 /* RGB */, img_size, f);
        if (pix_written != img_size)
        {
            printf("Failed to write pixels\n");
            ret = fclose(f);
            if (ret < 0)
            {
                printf("Failed to close file: %s\n", strerror(ret));
            }
            return -1;
        }
        break;
    }

    ret = fclose(f);
    if (ret < 0)
    {
        printf("Failed to close file: %s\n", strerror(ret));
    }
    return 0;
}
