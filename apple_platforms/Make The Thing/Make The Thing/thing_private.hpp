//
//  thing_private.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 2/20/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef thing_private_h
#define thing_private_h

//#include "drawtalk_parse.hpp"
#include "drawtalk.hpp"

#include "drawtalk_behavior.hpp"
#include "render_layer_labels.hpp"

#include "word_info.hpp"


#include "drawtalk_world.hpp"

//#include <type_traits>
#include "easings.h"

#define WITH_THING(name__, property_name__, block__ ) \
for (mtt::Thing_Ref* source = mtt::access<mtt::Thing_Ref>(thing, property_name__ ); source != nullptr;) { \
Thing* name__ = nullptr; if (source->try_get(& name__ )) { block__ } break; }

#define ALIGN_UP    vec3(0.0f, -0.5f, 0.0f)
#define ALIGN_DOWN  vec3(0.0f, 0.5f, 0.0f)
#define ALIGN_LEFT  vec3(-0.5f, 0.0f, 0.0f)
#define ALIGN_RIGHT vec3(0.5f, 0.0f, 0.0f)


#endif /* thing_private_h */
