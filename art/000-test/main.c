#include "amiss.h"
#include <stdio.h>
#include <string.h>

int main()
{
    uint8_t test[1080U * 1080U * 3U /* RGB */];
    memset(test, 150U, sizeof(test));
    amiss_img_st img = {
        .b = test,
        .blen = sizeof(test),
        .fmt = AMISS_IMG_FMT_PPM,
        .h = 1080U,
        .w = 1080U,
    };
    return amiss_img_save(&img, "000_test.ppm");
}
