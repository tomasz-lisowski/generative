#include "amiss.h"
#include "plutovg.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* How the rewritten word should grow on overflow. */
#define WLEN_SIZE_INIT 1024U
#define WLEN_SIZE_REALLOC 1024U

/* 0U for raster, 1U for vector. */
#define RASTER_OR_VECTOR 0U

/* Size of the image (it's a square). */
#define IMG_SIZE 1000U

typedef struct lsystem_draw_params_s
{
    double_t const angle_delta;
    double_t const angle_start;
    double_t const line_width_start;
    double_t const line_width_delta;
    double_t const line_width_min;
    double_t const line_len;
    uint32_t const x_start;
    uint32_t const y_start;
} lsystem_draw_params_st;

typedef struct lsystem_prule_s
{
    char const *const l;
    char const *const r;
    uint32_t const llen;
    uint32_t const rlen;
} lsystem_prule_st;

typedef struct lsystem_cword_s
{
    uint32_t const wlen;
    char const *const w;
} lsystem_cword_st;

typedef struct lsystem_vword_s
{
    uint32_t blen; /* Length of the allocated buffer. */
    /**
     * Length of the word contained in the buffer.
     * Should be <=blen.
     */
    uint32_t wlen;
    char *w;
} lsystem_vword_st;

typedef struct lsystem_s
{
    lsystem_cword_st const alph;
    lsystem_cword_st const axiom;
    uint8_t iters;

    /* Production rules. */
    uint32_t const pr_count;
    lsystem_prule_st const (*const pr)[];
} lsystem_st;

/**
 * @brief Determine if a production rule is applicable at given position in a
 * word.
 * @param prule Production rule to check.
 * @param word Word to check applicability of rule for.
 * @param word_idx Position in the word where to check if production rule
 * applies.
 * @return True if rule can be applied, false if not.
 */
bool lsystem_prule_check(lsystem_prule_st const prule,
                         lsystem_vword_st *const word, uint32_t const word_idx)
{
    if (word_idx + prule.llen <= word->wlen &&
        memcmp(&word->w[word_idx], prule.l, prule.llen) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Perform one rewrite iteration of the given word using a grammar.
 * @param grammar What L-System grammar to use for generation.
 * @param word Where the generated word will be stored.
 * @return 0 on success, 1 on failure.
 */
uint8_t lsystem_rewrite(lsystem_st const grammar, lsystem_vword_st *const word)
{
    /* Nothing to rewrite. */
    if (word->wlen == 0)
    {
        return 1;
    }
    uint32_t prules_applied_total = 0U;
    for (uint32_t word_idx = 0U; word_idx < word->wlen; ++word_idx)
    {
        uint32_t prules_applied = 0U;
        for (uint32_t prule_idx = 0U; prule_idx < grammar.pr_count; ++prule_idx)
        {
            lsystem_prule_st const prule = (*grammar.pr)[prule_idx];
            if (lsystem_prule_check(prule, word, word_idx) == true)
            {
                log_dbg("RWR",
                        // clang-format off
                        "Applying rule %.*s->%.*s"
                        CLR_TXT(CLR_YEL, " = ")
                        "%.*s"
                        CLR_TXT(CLR_BLU, "%.*s")
                        "%.*s"
                        " to...\n",
                        // clang-format on
                        prule.llen, prule.l, prule.rlen, prule.r, word_idx,
                        &word->w[0U], prule.llen, &word->w[word_idx],
                        word->wlen - word_idx - prule.llen,
                        &word->w[word_idx + prule.llen]);

                int64_t const wlen_delta = prule.rlen - prule.llen;
                /**
                 * Safe cast because it is not possible for word length to
                 * become negative here.
                 */
                uint32_t const wlen_after = (uint32_t)(word->wlen + wlen_delta);

                /**
                 * Allocate extra memory for word if rewrite rule leads to
                 * overflow of current word buffer.
                 */
                if (wlen_after > word->blen)
                {
                    uint32_t const blen_need = wlen_after;
                    uint32_t const blen_new =
                        word->blen + WLEN_SIZE_REALLOC > blen_need
                            ? word->blen + WLEN_SIZE_REALLOC
                            : blen_need;
                    char *w_new = realloc(word->w, blen_new);
                    if (w_new == NULL)
                    {
                        log_err(
                            "RWR",
                            "Failed to realloc buffer for rewritten word\n");
                        return 1U;
                    }
                    log_dbg("RWR", "Reallocated word buffer from %u to %u\n",
                            word->blen, blen_new);
                    word->w = w_new;
                    word->blen = blen_new;
                }

                /**
                 * In order to rewrite, might have to move parts of the current
                 * word to not overwrite them.
                 */
                if (wlen_delta != 0)
                {
                    memmove(&word->w[word_idx + (prule.rlen - prule.llen) + 1U],
                            &word->w[word_idx + prule.llen],
                            word->wlen - word_idx - prule.llen);
                }

                /**
                 * Write RHS of production rule where LHS is located in the
                 * word.
                 */
                if (prule.rlen > 0)
                {
                    memcpy(&word->w[word_idx], prule.r, prule.rlen);
                }

                log_dbg("RWR", "%.*s" CLR_TXT(CLR_BLU, "%.*s") "%.*s\n",
                        word_idx, &word->w[0U], prule.rlen, &word->w[word_idx],
                        word->wlen - word_idx - prule.rlen,
                        &word->w[word_idx + prule.rlen]);

                int64_t const word_idx_new = word_idx + wlen_delta;
                if (word_idx_new < 0 || word_idx_new > UINT32_MAX)
                {
                    log_dbg("RWR", "Word index cannot become negative\n");
                    return 1U;
                }
                word_idx =
                    (uint32_t)(word_idx +
                               wlen_delta); /* Safe cast due to bound check. */
                word->wlen = wlen_after;
                prules_applied++;
            }
        }
        prules_applied_total += prules_applied;
    }
    if (prules_applied_total == 0U)
    {
        return 1;
    }
    return 0U;
}

/**
 * @brief Draw a word generated using an L-System (F,+,-,[,]).
 * @param word The word to draw.
 * @param draw_params How to draw the word.
 * @param img Image to rasterize word on.
 * @param pluto Context for when using vector library.
 * @return 0 on success, 1 on failure.
 */
uint8_t lsystem_draw(lsystem_vword_st const word,
                     lsystem_draw_params_st const draw_params,
#if RASTER_OR_VECTOR == 1U
                     plutovg_t *const pluto
#else
                     amiss_img_st const img
#endif
)
{
    color_st const color_branch = {.a = {150U, 166U, 50U}};
    uint32_t const stack_size = 1024U;
    double_t const line_width = draw_params.line_width_min;
    double_t const line_len = draw_params.line_len;
    double_t const angle_delta = draw_params.angle_delta;
    vec2u32_st *pos_stack = malloc(stack_size * sizeof(vec2u32_st));
    double_t *angle_stack = malloc(stack_size * sizeof(double_t));
    double_t *width_delta_stack = malloc(stack_size * sizeof(double_t));
    uint32_t sp = 0U;

    pos_stack[0U].x = draw_params.x_start;
    pos_stack[0U].y = draw_params.y_start;
    angle_stack[0U] = 180.0 + 90.0 + draw_params.angle_start; /* Degrees. */
    width_delta_stack[0U] = draw_params.line_width_start;

    for (uint32_t word_idx = 0; word_idx < word.wlen; ++word_idx)
    {
        switch (word.w[word_idx])
        {
        case 'F': {
            vec2u32_st const start = {.x = pos_stack[sp].x,
                                      .y = pos_stack[sp].y};
            int64_t endx =
                pos_stack[sp].x +
                llround(cos(angle_stack[sp] * (M_PI / 180.0)) * line_len);
            int64_t endy =
                pos_stack[sp].y +
                llround(sin(angle_stack[sp] * (M_PI / 180.0)) * line_len);
            if (endx < 0)
            {
                endx = 0;
            }
            if (endy < 0)
            {
                endy = 0;
            }
            vec2u32_st const end = {
                .x = (uint32_t)endx,
                .y = (uint32_t)endy}; /* Safe cast thanks to bound checks. */
#if RASTER_OR_VECTOR == 1U
            plutovg_move_to(pluto, start.x, start.y);
            plutovg_line_to(pluto, end.x, end.y);
            plutovg_set_source_rgb(pluto, color_branch.r / 255.0,
                                   color_branch.g / 255.0,
                                   color_branch.b / 255.0);
            plutovg_set_line_width(pluto, width_delta_stack[sp] > line_width
                                              ? width_delta_stack[sp]
                                              : line_width);
            plutovg_stroke(pluto);
#else
            amiss_draw_line(&img, color_branch, line_width, false, start, end);
#endif
            width_delta_stack[sp] -= draw_params.line_width_delta;
            pos_stack[sp].x = end.x;
            pos_stack[sp].y = end.y;
            break;
        }
        case '+': {
            angle_stack[sp] = fmod(angle_stack[sp] + angle_delta, 365.0);
            break;
        }
        case '-': {
            double_t const angle_next = angle_stack[sp] - angle_delta;
            angle_stack[sp] =
                angle_next < 0.0 ? 365.0 - angle_next : angle_next;
            break;
        }
        case '[': {
            if (sp + 1U >= stack_size)
            {
                log_err("DRAW", "Stack too small\n");
                free(pos_stack);
                free(angle_stack);
                free(width_delta_stack);
                return 1;
            }
            memcpy(&pos_stack[sp + 1U], &pos_stack[sp], sizeof(pos_stack[0U]));
            memcpy(&angle_stack[sp + 1U], &angle_stack[sp],
                   sizeof(angle_stack[0U]));
            memcpy(&width_delta_stack[sp + 1U], &width_delta_stack[sp],
                   sizeof(width_delta_stack[0U]));
            sp += 1;
            break;
        }
        case ']': {
            if ((int64_t)sp - 1 < 0)
            {
                log_err("DRAW", "Stack too small\n");
                free(pos_stack);
                free(angle_stack);
                free(width_delta_stack);
                return 1U;
            }
            sp -= 1;
            break;
        }

        default:
            break;
        }
    }
    free(pos_stack);
    free(angle_stack);
    free(width_delta_stack);
    return 0U;
}

/**
 * @brief Generate a word using an L-system definition and draw it according to
 * drawing params.
 * @param ls The L-system to use.
 * @param draw_params How to draw the word after it is generated.
 * @param path_out Where to save the drawn word.
 * @return 0 on success, 1 on failure.
 */
uint8_t lsystem_gen(lsystem_st const ls,
                    lsystem_draw_params_st const draw_params,
                    char const *const path_out)
{
    /* Gradient details. */
    double_t stops[] = {0.0, 0.5, 1.0};
    color_st colors[] = {
        {.a = {247U, 248U, 239U}},
        {.a = {213U, 219U, 173U}},
        {.a = {225U, 230U, 196U}},
    };

#if RASTER_OR_VECTOR == 1U
    plutovg_surface_t *const pluto_surface =
        plutovg_surface_create(IMG_SIZE, IMG_SIZE);
    plutovg_t *const pluto = plutovg_create(pluto_surface);

    /* Add background to image. */
    plutovg_gradient_t *const gradient = plutovg_gradient_create_linear(
        IMG_SIZE / 2U, 0U, IMG_SIZE / 2U, IMG_SIZE);
    for (uint8_t stop_idx = 0U; stop_idx < sizeof(stops) / sizeof(stops[0]);
         ++stop_idx)
    {
        plutovg_gradient_add_stop_rgb(
            gradient, stops[stop_idx], colors[stop_idx].r / 255.0,
            colors[stop_idx].g / 255.0, colors[stop_idx].b / 255.0);
    }
    plutovg_rect(pluto, 0U, 0U, IMG_SIZE, IMG_SIZE);
    plutovg_set_source_gradient(pluto, gradient);
    plutovg_fill(pluto);
#else
    uint32_t img_buf_len = IMG_SIZE * IMG_SIZE * 3U /* RGB */;
    uint8_t *img_buf = malloc(img_buf_len);
    amiss_img_st img = {
        .b = img_buf,
        .blen = img_buf_len,
        .fmt = AMISS_IMG_FMT_PPM,
        .h = IMG_SIZE,
        .w = IMG_SIZE,
    };

    /* Add background to image. */
    gradient_st gradient = {.count = 3U, .colors = colors, .stops = stops};
    amiss_draw_bg_gradient(&img, gradient);
#endif

    lsystem_vword_st word = {.w = malloc(WLEN_SIZE_INIT),
                             .wlen = ls.axiom.wlen,
                             .blen = WLEN_SIZE_INIT};
    if (word.w == NULL)
    {
        log_err("MAIN", "Failed to allocate word\n");
        return 1U;
    }
    memcpy(word.w, ls.axiom.w, ls.axiom.wlen);

    uint32_t const iter_max = ls.iters;
    for (uint32_t iter = 0U; iter <= iter_max + 1; ++iter)
    {
        /* Do something with intermediate word. */
        log_info("MAIN", "[%u] '%.*s'\n\n", iter, word.wlen, word.w);
        if (iter + 1 > iter_max + 1)
        {
            break;
        }

        int ret = lsystem_rewrite(ls, &word);
        if (ret > 0)
        {
            log_info("MAIN", "No more rules can be applied\n");
            break;
        }
        else if (ret < 0)
        {
            log_err("MAIN", "An error occurred\n");
            return 1U;
        }
    }
#if RASTER_OR_VECTOR == 1U
    lsystem_draw(word, draw_params, pluto);
    plutovg_surface_write_to_png(pluto_surface, path_out);
    plutovg_gradient_destroy(gradient);
    plutovg_surface_destroy(pluto_surface);
    plutovg_destroy(pluto);
#else
    lsystem_draw(word, draw_params, img);
    if (amiss_img_save(&img, path_out) != 0)
    {
        log_err("MAIN", "Failed to save image to disk\n");
    }
    free(img.b);
#endif
    free(word.w);
    return 0U;
}

int main()
{
    /* Generate L-system. */
    lsystem_draw_params_st const draw_params[] = {
        {
            .angle_delta = 30.0,
            .angle_start = 0.0,
            .line_width_delta = 0.04,
            .line_width_start = 6.0,
            .line_width_min = 1.2,
            .line_len = 2.4,
            .x_start = 520U,
            .y_start = IMG_SIZE - 1U,
        },
        {
            .angle_delta = 24.5,
            .angle_start = 0.0,
            .line_width_delta = 0.05,
            .line_width_start = 2.0,
            .line_width_min = 0.5,
            .line_len = 8.0,
            .x_start = 580U,
            .y_start = IMG_SIZE - 1U,
        },
        {
            .angle_delta = 20.0,
            .angle_start = -2.0,
            .line_width_delta = 0.08,
            .line_width_start = 8.0,
            .line_width_min = 1.0,
            .line_len = 7.0,
            .x_start = 460U,
            .y_start = IMG_SIZE - 1U,
        },
    };
    lsystem_prule_st const prule[][2U] = {
        {
            {
                .l = "X",
                .r = "F+[[X]-X]-F[-FX]+X",
                .llen = 1U,
                .rlen = 18U,
            },
            {
                .l = "F",
                .r = "FF",
                .llen = 1U,
                .rlen = 2U,
            },
        },
        {
            {
                .l = "F",
                .r = "FF-[-F+F+F]+[+F-F-F]",
                .llen = 1U,
                .rlen = 20U,
            },
        },
        {
            {
                .l = "X",
                .r = "F[+X]F[-X]+X",
                .llen = 1U,
                .rlen = 12U,
            },
            {
                .l = "F",
                .r = "FF",
                .llen = 1U,
                .rlen = 2U,
            },
        },
    };
    lsystem_st const ls[] = {
        {
            .alph =
                {
                    .w = "XF+-[]",
                    .wlen = 6U,
                },
            .axiom =
                {
                    .w = "X",
                    .wlen = 1U,
                },
            .iters = 6U,
            .pr_count = 2U,
            .pr = &prule[0U],
        },
        {
            .alph =
                {
                    .w = "F+-[]",
                    .wlen = 5U,
                },
            .axiom =
                {
                    .w = "F",
                    .wlen = 1U,
                },
            .iters = 4U,
            .pr_count = 1U,
            .pr = &prule[1U],
        },
        {
            .alph =
                {
                    .w = "XF+-[]",
                    .wlen = 6U,
                },
            .axiom =
                {
                    .w = "X",
                    .wlen = 1U,
                },
            .iters = 5U,
            .pr_count = 2U,
            .pr = &prule[2U],
        },
    };

#if RASTER_OR_VECTOR == 1U
    lsystem_gen(ls[0U], draw_params[0U], "001_lsystem_rule0.png");
    lsystem_gen(ls[1U], draw_params[1U], "001_lsystem_rule1.png");
    lsystem_gen(ls[2U], draw_params[2U], "001_lsystem_rule2.png");
#else
    lsystem_gen(ls[0U], draw_params[0U], "001_lsystem_rule0.ppm");
    lsystem_gen(ls[1U], draw_params[1U], "001_lsystem_rule1.ppm");
    lsystem_gen(ls[2U], draw_params[2U], "001_lsystem_rule2.ppm");
#endif

    return EXIT_SUCCESS;
}
