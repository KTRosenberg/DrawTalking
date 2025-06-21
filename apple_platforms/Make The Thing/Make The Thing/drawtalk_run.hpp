//
//  drawtalktalk_run.hpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 8/13/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_run_hpp
#define drawtalk_run_hpp


#include "drawtalk.hpp"
//#include "cpp_common.hpp"
#include "job_dispatch.hpp"
#include "drawtalk_world.hpp"

namespace dt::old {

enum struct MTT_NODISCARD Run_Status {
    OK,
    FAILED,
};

cstring Run_Status_Strings[] = {
    [(uint64)Run_Status::OK]     = "OK",
    [(uint64)Run_Status::FAILED] = "FAILED",
};

struct Run_Return {
    void* data = nullptr;
};

using  Run_Result = mtt::Result<Run_Return, Run_Status>;
inline Run_Result run_failed() { return (Run_Result){ .status = Run_Status::FAILED}; };
inline Run_Result run_ok(void* data) { return (Run_Result){ .status = Run_Status::OK, .value = (Run_Return){ .data = data }}; };


struct Entities {
    mtt::String type;
    bool        is_random ;
    bool        is_type   ;
    bool        is_numeric;
};


struct T_R {
    Speech_Property* container;
    Speech_Property::Prop_List* triggers;
    Speech_Property::Prop_List* responses;
};

Run_Result run(dt::DrawTalk* dt_ctx);

}

#endif /* drawtalk_run_hpp */
