//
//  c_library.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/14/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef c_library_h
#define c_library_h

#include "c_common.h"
#include "twn.h"
#include "time_common.h"
#include "stb_perlin.h"

#include "pcg_variants.h"

#include "string_intern.h"

typedef struct MTT_Random_Number_Generation {
    pcg64_random_t state;
} MTT_Random_Number_Generation;

typedef MTT_Random_Number_Generation MTT_RNG;

extern MTT_Random_Number_Generation mtt_rng;

void MTT_Random_init(uint64 init_state);

MTT_Random_Number_Generation MTT_Random_init_own(uint64 init_state);

sint64 MTT_Random_signed(void);

uint64 MTT_Random(void);

sint64 MTT_Random_lower_bounded(sint64 lower_bound_exclusive);

sint64 MTT_Random_range(sint64 lower_bound_inclusive, sint64 upper_bound_exclusive);

// returns [0..1) double
float64 MTT_Random_Float64_value(void);

// returns [0..scale) double
float64 MTT_Random_Float64_value_scaled(float64 scale);

void MTT_Random_Float64_value_within_circle(float64 extent, float64* radius_out, float64* angle_radians_out);

#endif /* c_library_h */
