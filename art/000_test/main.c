#include "amiss.h"
#include <stdio.h>
#include <string.h>

int main()
{
    uint8_t test[1080 * 1080 * 3] = {0};
    memset(test, 150, sizeof(test));
    amiss_img_st img = {
        .b = test,
        .blen = sizeof(test),
        .fmt = AMISS_IMG_FMT_PPM,
        .h = 1080,
        .w = 1080,
    };
    amiss_img_save(&img, "test.ppm");
    return 0;
}
