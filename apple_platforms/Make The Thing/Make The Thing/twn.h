#ifndef TWN_H
#define TWN_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    void *ptr;
    double start, end;
    double cur, step;
    double delay;
    char ease, type;
    char do_forwards_and_backwards;
    void (*callback)(void*);
    void* user_data;
} twn_Tween;

typedef struct {
    double delay;
    double time;
    char ease;
    char do_forwards_and_backwards;
    char abort_if_in_progress;
    void (*callback)(void*);
    void* user_data;
} twn_Opt;

typedef struct {
    int capacity;
} twn_Context;

enum {
    TWN_LINEAR,
    TWN_QUADIN,
    TWN_QUADOUT,
    TWN_QUADINOUT,
    TWN_QUARTIN,
    TWN_QUARTOUT,
    TWN_QUARTINOUT,
    TWN_QUINTIN,
    TWN_QUINTOUT,
    TWN_QUINTINOUT,
    TWN_BACKIN,
    TWN_BACKOUT,
    TWN_BACKINOUT,
};

twn_Context* twn_new(int capacity);
twn_Context* twn_new_from_buffer(void *buf, int buf_sz);
void twn_free(twn_Context *ctx);
void twn_update(twn_Context *ctx, double dt);
void twn_clear(twn_Context *ctx, void *ptr, int sz);
void twn_clear_all(twn_Context *ctx);
int twn_get_active_count(twn_Context *ctx);

void twn_add_f64(twn_Context *ctx, double *val, float dst, twn_Opt opt);
void twn_add_f32(twn_Context *ctx, float *val, float dst, twn_Opt opt);
void twn_add_i64(twn_Context *ctx, int64_t *val, float dst, twn_Opt opt);
void twn_add_i32(twn_Context *ctx, int32_t *val, float dst, twn_Opt opt);
void twn_add_u8(twn_Context *ctx, uint8_t *val, float dst, twn_Opt opt);

#endif // TWN_H


//////////////////////////////////////////////////////////////////////////////

#ifdef TWN_IMPL


#define twn_align_up(num, align) \
    (((num) + ((align) - 1)) & ~((align) - 1))

enum { TWN__F64, TWN__F32, TWN__I64, TWN__I32, TWN__U8 };


static twn_Tween* twn__get_tweens(twn_Context *ctx) {
    return (twn_Tween*) (twn_align_up(((uintptr_t)ctx + 1), 16));
}

static void twn__set_number(int type, void *ptr, double n) {
    switch (type) {
        case TWN__F64 : *((double*)  ptr) = n; break;
        case TWN__F32 : *((float*)   ptr) = n; break;
        case TWN__I64 : *((int64_t*) ptr) = n; break;
        case TWN__I32 : *((int32_t*) ptr) = n; break;
        case TWN__U8  : *((uint8_t*) ptr) = n; break;
    }
}

static double twn__get_number(int type, void *ptr) {
    switch (type) {
        case TWN__F64 : return *((double*)  ptr);
        case TWN__F32 : return *((float*)   ptr);
        case TWN__I64 : return *((int64_t*) ptr);
        case TWN__I32 : return *((int32_t*) ptr);
        case TWN__U8  : return *((uint8_t*) ptr);
    }
    return 0;
}


static double twn__ease_in(int ease, double p) {
    switch (ease) {
        case TWN_QUADIN  : return p * p;
        case TWN_QUARTIN : return p * p * p;
        case TWN_QUINTIN : return p * p * p * p;
        case TWN_BACKIN  : return p * p * (2.7 * p - 1.7);
    }
    return p;
}

static double twn__ease_out(int ease_in, double p) {
    return 1 - twn__ease_in(ease_in, 1 - p);
}

static double twn__ease_inout(int ease_in, double p) {
    p *= 2;
    if (p < 1) {
        return 0.5 * twn__ease_in(ease_in, p);
    }
    p = 2 - p;
    return 0.5 * (1 - twn__ease_in(ease_in, p)) + 0.5;
}


static double twn__ease(int ease, double p) {
    switch (ease) {
        case TWN_LINEAR     : return p;
        case TWN_QUADIN     : return twn__ease_in    ( TWN_QUADIN,  p );
        case TWN_QUADOUT    : return twn__ease_out   ( TWN_QUADIN,  p );
        case TWN_QUADINOUT  : return twn__ease_inout ( TWN_QUADIN,  p );
        case TWN_QUARTIN    : return twn__ease_in    ( TWN_QUARTIN, p );
        case TWN_QUARTOUT   : return twn__ease_out   ( TWN_QUARTIN, p );
        case TWN_QUARTINOUT : return twn__ease_inout ( TWN_QUARTIN, p );
        case TWN_QUINTIN    : return twn__ease_in    ( TWN_QUINTIN, p );
        case TWN_QUINTOUT   : return twn__ease_out   ( TWN_QUINTIN, p );
        case TWN_QUINTINOUT : return twn__ease_inout ( TWN_QUINTIN, p );
        case TWN_BACKIN     : return twn__ease_in    ( TWN_BACKIN,  p );
        case TWN_BACKOUT    : return twn__ease_out   ( TWN_BACKIN,  p );
        case TWN_BACKINOUT  : return twn__ease_inout ( TWN_BACKIN,  p );
    }
    return p;
}


twn_Context* twn_new(int capacity) {
    int n = twn_align_up(sizeof(twn_Context), 16) + (capacity * (twn_align_up(sizeof(twn_Tween), 16)));
    return twn_new_from_buffer(aligned_alloc(16, twn_align_up(n, 16)), twn_align_up(n, 16));
}


twn_Context* twn_new_from_buffer(void *buf, int buf_sz) {
    memset(buf, 0, buf_sz);
    twn_Context *ctx = buf;
    ctx->capacity = (buf_sz - sizeof(twn_Context)) / sizeof(twn_Tween);
    return ctx;
}


void twn_free(twn_Context *ctx) {
    free(ctx);
}


static void twn__update_tween(twn_Tween *t, double dt) {
    if (t->delay > 0) {
        t->delay -= dt;
        return;
    }

    t->cur += t->step * dt;

    if (t->cur >= 1.0) {
        twn__set_number(t->type, t->ptr, t->end);
        if (t->do_forwards_and_backwards != 0) {
            t->do_forwards_and_backwards = 0;
            
            double tmp = t->start;
            t->start = t->end;
            t->end = tmp;
        } else {
            if (t->callback != NULL) {
                t->callback(t->user_data);
            }
            t->ptr = NULL;
        }
        return;
    }

    double p = twn__ease(t->ease, t->cur);
    twn__set_number(t->type, t->ptr, t->start + p * (t->end - t->start));
}


void twn_update(twn_Context *ctx, double dt) {
    twn_Tween *tweens = twn__get_tweens(ctx);
    for (int i = 0; i < ctx->capacity; i++) {
        twn_Tween *t = &tweens[i];
        if (!t->ptr) { continue; }
        twn__update_tween(t, dt);
    }
}


void twn_clear(twn_Context *ctx, void *ptr, int sz) {
    void *end_ptr = ((char*) ptr) + sz;
    twn_Tween *tweens = twn__get_tweens(ctx);
    for (int i = 0; i < ctx->capacity; i++) {
        twn_Tween *t = &tweens[i];
        if (t->ptr >= ptr && t->ptr < end_ptr) {
            tweens[i].ptr = NULL;
        }
    }
}


void twn_clear_all(twn_Context *ctx) {
    twn_Tween *tweens = twn__get_tweens(ctx);
    for (int i = 0; i < ctx->capacity; i++) {
        tweens[i].ptr = NULL;
    }
}


int twn_get_active_count(twn_Context *ctx) {
    int count = 0;
    twn_Tween *tweens = twn__get_tweens(ctx);
    for (int i = 0; i < ctx->capacity; i++) {
        if (tweens[i].ptr) { count++; }
    }
    return count;
}


void twn__add(twn_Context *ctx, int type, void *ptr, float dst, twn_Opt opt) {
    twn_Tween *tweens = twn__get_tweens(ctx);
    twn_Tween *tween = NULL;

    // get tween for ptr or an unused tween if there is no existing one
    for (int i = 0; i < ctx->capacity; i++) {
        twn_Tween *t = &tweens[i];
        if (t->ptr == ptr) {
            if (opt.do_forwards_and_backwards ||
                opt.abort_if_in_progress) {
                return;
            }
            tween = t;
            break;
        }
        if (tween == NULL && t->ptr == NULL) {
            tween = t;
        }
    }

    // handle failure to retrieve a free tween from the pool or time being 0
    if (tween == NULL || opt.time == 0) {
        twn__set_number(type, ptr, dst);
        return;
    }

    // init tween
    *tween = (twn_Tween) {
        .type  = type,
        .ptr   = ptr,
        .start = twn__get_number(type, ptr),
        .end   = dst,
        .step  = 1.0 / ((opt.do_forwards_and_backwards != 0) ? 0.5 * opt.time : opt.time),
        .ease  = opt.ease,
        .delay = opt.delay,
        
        .do_forwards_and_backwards = opt.do_forwards_and_backwards,
        .callback = opt.callback,
        .user_data = opt.user_data,
    };
}


void twn_add_f64(twn_Context *ctx, double *val, float dst, twn_Opt opt) {
    twn__add(ctx, TWN__F64, val, dst, opt);
}

void twn_add_f32(twn_Context *ctx, float *val, float dst, twn_Opt opt) {
    twn__add(ctx, TWN__F32, val, dst, opt);
}

void twn_add_i64(twn_Context *ctx, int64_t *val, float dst, twn_Opt opt) {
    twn__add(ctx, TWN__I64, val, dst, opt);
}

void twn_add_i32(twn_Context *ctx, int32_t *val, float dst, twn_Opt opt) {
    twn__add(ctx, TWN__I32, val, dst, opt);
}

void twn_add_u8(twn_Context *ctx, uint8_t *val, float dst, twn_Opt opt) {
    twn__add(ctx, TWN__U8, val, dst, opt);
}

#endif // TWN_IMPL
