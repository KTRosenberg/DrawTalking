//
//  c_library.c
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/14/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//
#define TWN_IMPL
#define STB_PERLIN_IMPLEMENTATION

#include "c_library.h"

MTT_RNG mtt_rng;

void MTT_Random_init(uint64 init_state)
{
    
    //        pcg64_srandom_r(rngptr, initstate, initseq)
    //        This function initializes (a.k.a. “seeds”) the random number generator, a required initialization step before the generator can be used. The provided arguments are defined as follows:
    //
    //        rngptr should point to the address of a pcg32_random_t value that you've previously declared
    //        initstate is the starting state for the RNG, you can pass any 64-bit value.
    //        initseq selects the output sequence for the RNG, you can pass any 64-bit value, although only the low 63 bits are significant.
            

    // https://www.pcg-random.org/using-pcg-c-basic.html
    pcg64_srandom_r(&mtt_rng.state, init_state, (intptr_t)&mtt_rng.state);
}

MTT_Random_Number_Generation MTT_Random_init_own(uint64 init_state)
{
    MTT_Random_Number_Generation rng = {};
    pcg64_srandom_r(&rng.state, init_state, (intptr_t)&rng.state);
    return rng;
}

sint64 MTT_Random_signed(void)
{
    return (sint64)pcg64_random_r(&mtt_rng.state);
}
uint64 MTT_Random(void)
{
    return (uint64)pcg64_random_r(&mtt_rng.state);
}

sint64 MTT_Random_lower_bounded(sint64 lower_bound_exclusive)
{
    return (sint64)pcg64_boundedrand_r(&mtt_rng.state, lower_bound_exclusive);
}

sint64 MTT_Random_range(sint64 lower_bound_inclusive, sint64 upper_bound_exclusive)
{
    return (sint64)pcg64_boundedrand_r(&mtt_rng.state, upper_bound_exclusive - lower_bound_inclusive) + lower_bound_inclusive;
}

// returns [0..1) double
float64 MTT_Random_Float64_value(void) {
    return 0x1.0p-53 * ((MTT_Random())  >> 11);
}
// returns [0..scale) double
float64 MTT_Random_Float64_value_scaled(float64 scale) {
    return scale * MTT_Random_Float64_value();
}

void MTT_Random_Float64_value_within_circle(float64 extent, float64* radius_out, float64* angle_radians_out)
{
    *radius_out = extent * sqrt(MTT_Random_Float64_value());
    *angle_radians_out = MTT_Random_Float64_value_scaled(M_PI * 2);
}
                               
