//
//  augmented_reality.hpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/30/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef augmented_reality_hpp
#define augmented_reality_hpp

#define USE_ARKIT (true)

namespace mtt {

typedef enum AUGMENTED_REALITY_FLAG {
    AUGMENTED_REALITY_FLAG_FACE_TRACKING  = (1 << 0),
    AUGMENTED_REALITY_FLAG_WORLD_TRACKING = (1 << 1),
} AUGMENTED_REALITY_FLAG;

struct Augmented_Reality_Context {
    void*  backend_ctx;
    void*  backend_host;
    uint64 flags;
    bool   is_active;
    bool   is_loaded;
    bool   is_supported;
    bool   freeze_frame_when_off;
};

bool Augmented_Reality_load(Augmented_Reality_Context* ctx);
void Augmented_Reality_unload(Augmented_Reality_Context* ctx);

bool Augmented_Reality_run(Augmented_Reality_Context* ctx);
void Augmented_Reality_pause(Augmented_Reality_Context* ctx);

void Augmented_Reality_config(Augmented_Reality_Context* ctx, uint64 flags);



}

#endif /* augmented_reality_hpp */
