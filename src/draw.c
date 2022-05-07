#include "amiss.h"
#include <stdlib.h>

__attribute__((unused)) static void color_antialias(
    amiss_img_st const *const img, color_st *const color_aa,
    color_st const color, color_st const color_bg, double_t const antialias)
{
    uint8_t const depth = amiss_img_depth(img);
    for (uint8_t depth_idx = 0U; depth_idx < depth; ++depth_idx)
    {
        (*color_aa).a[depth_idx] =
            (uint8_t)(((1 - antialias) * color.a[depth_idx]) +
                      (antialias * color_bg.a[depth_idx]));
    }
}

/**
 * @brief General purpose bresenham implementation. It takes in a callback which
 * gets called for every pixel traced by this algorithm.
 * @param img An image to draw the line on.
 * @param start Where the line should start.
 * @param end Where the line should end.
 * @return Length of the line.
 */
static uint32_t bresenham_thin(amiss_img_st const *const img,
                               color_st const color, vec2u32_st const start,
                               vec2u32_st const end)
{
    if (start.x >= img->w || end.x >= img->w || start.y >= img->h ||
        end.y >= img->h)
    {
        return 0;
    }

    int64_t dx = llabs((int64_t)end.x - (int64_t)start.x),
            sx = start.x < end.x ? 1 : -1;
    int64_t dy = -llabs((int64_t)end.y - (int64_t)start.y),
            sy = start.y < end.y ? 1 : -1;
    int64_t err = dx + dy, e2; /* error value e_xy */

    uint32_t x = start.x;
    uint32_t y = start.y;

    uint16_t length = 0;

    for (;;)
    {
        amiss_draw_px_set(img, color, x, y);
        if (x == end.x && y == end.y)
        {
            break;
        }
        length++;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            if (x + sx < 0 || x + sx > UINT32_MAX)
            {
                return 0;
            }
            x = (uint32_t)(x + sx); /* Safe cast due to bounds check. */
        }                           /* e_xy+e_x > 0 */
        if (e2 <= dx)
        {
            err += dx;
            if (y + sy < 0 || y + sy > UINT32_MAX)
            {
                return 0;
            }
            y = (uint32_t)(y + sy); /* Safe cast due to bounds check. */
        }                           /* e_xy+e_y < 0 */
    }
    return length;

    return length;
}

inline void amiss_draw_px_set(amiss_img_st const *const img,
                              color_st const color, uint32_t const x,
                              uint32_t const y)
{
    if (x >= img->w || y >= img->h)
    {
        return; /* Outside of the image. */
    }
    uint8_t const depth = amiss_img_depth(img);
    for (uint8_t depth_idx = 0; depth_idx < depth; ++depth_idx)
    {
        img->b[amiss_img_xy2idx(img, depth, x, y) + depth_idx] =
            color.a[depth_idx];
    }
}

inline void amiss_draw_px_get(amiss_img_st const *const img,
                              color_st *const color, uint32_t const x,
                              uint32_t const y)
{
    if (x >= img->w || y >= img->h)
    {
        return; /* Outside of the image. */
    }
    uint8_t const depth = amiss_img_depth(img);
    for (uint8_t depth_idx = 0; depth_idx < depth; ++depth_idx)
    {
        (*color).a[depth_idx] =
            img->b[amiss_img_xy2idx(img, depth, x, y) + depth_idx];
    }
}

uint32_t amiss_draw_line(amiss_img_st const *const img, color_st const color,
                         __attribute__((unused)) double_t const thickness,
                         __attribute__((unused)) bool const antialias,
                         vec2u32_st const start, vec2u32_st const end)
{
    return bresenham_thin(img, color, start, end);
}

void amiss_draw_bg_gradient(amiss_img_st const *const img,
                            gradient_st const gradient)
{
    if (gradient.count == 0)
    {
        return;
    }
    for (uint32_t stop_idx = 0U; stop_idx < gradient.count; ++stop_idx)
    {
        if (gradient.stops[stop_idx] < 0.0 || gradient.stops[stop_idx] > 1.0)
        {
            return; /* Stops can't be negative or greater than 1. */
        }
    }

    /* Fill in space before first stop with first color. */
    uint32_t const y_stop_pre =
        (uint32_t)(gradient.stops[0U] *
                   img->h) /* Safe cast due to bound checks on stops. */;
    for (uint32_t y_seg = 0U; y_seg < y_stop_pre; ++y_seg)
    {
        for (uint32_t x_seg = 0U; x_seg < img->w; ++x_seg)
        {
            amiss_draw_px_set(img, gradient.colors[0U], x_seg, y_seg);
        }
    }

    /* Fill in space before after last stop with last color. */
    uint32_t const y_stop_post =
        (uint32_t)(gradient.stops[gradient.count - 1U] *
                   img->h) /* Safe cast due to bound checks on stops. */;
    for (uint32_t y_seg = y_stop_post; y_seg < img->h; ++y_seg)
    {
        for (uint32_t x_seg = 0U; x_seg < img->w; ++x_seg)
        {
            amiss_draw_px_set(img, gradient.colors[gradient.count - 1U], x_seg,
                              y_seg);
        }
    }

    /* Draw gradient. */
    uint32_t seg_idx = 0U;
    for (uint32_t y_seg = y_stop_pre; y_seg < img->h; ++y_seg)
    {
        /* Decide if next color should be used from this y onwards. */
        uint32_t const seg_stop =
            (uint32_t)lround(gradient.stops[seg_idx + 1U] * (double_t)img->h);
        if (y_seg >= seg_stop)
        {
            seg_idx++;
            if (seg_idx + 1U >= gradient.count)
            {
                break;
            }
        }

        uint32_t const seg_start =
            (uint32_t)lround(gradient.stops[seg_idx] * (double_t)img->h);
        uint32_t const seg_len = (uint32_t)lround(
            (gradient.stops[seg_idx + 1U] - gradient.stops[seg_idx]) *
            (double_t)img->h);
        double_t const frac_seg = (y_seg + 1.0 - seg_start) / (double_t)seg_len;
        double_t const frac_a = 1.0 - frac_seg;
        double_t const frac_b = frac_seg;
        color_st const color_a = gradient.colors[seg_idx];
        color_st const color_b = gradient.colors[seg_idx + 1U];
        color_st const color = {
            .r = (uint8_t)((frac_a * color_a.r) + (frac_b * color_b.r)),
            .g = (uint8_t)((frac_a * color_a.g) + (frac_b * color_b.g)),
            .b = (uint8_t)((frac_a * color_a.b) + (frac_b * color_b.b)),
        };
        log_dbg("AMISS_DRAW",
                "Drawing gradient line:"
                "\n    color %u and %u"
                "\n    seg_start = %u"
                "\n    seg_len = %u"
                "\n    y = %u"
                "\n    frac = %f\n",
                seg_idx, seg_idx + 1, seg_start, seg_len, y_seg, frac_seg);
        for (uint32_t x_seg = 0U; x_seg < img->w; ++x_seg)
        {
            amiss_draw_px_set(img, color, x_seg, y_seg);
        }
    }
}
