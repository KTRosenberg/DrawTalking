#ifndef FLECS_DUMP_H
#define FLECS_DUMP_H

#include "flecs.h"

namespace flecs {

void dump_indent(int count);

void dump(flecs::entity e, int indent = 0, bool is_instanceof = false);

void dump(flecs::iter it);


}

#endif
