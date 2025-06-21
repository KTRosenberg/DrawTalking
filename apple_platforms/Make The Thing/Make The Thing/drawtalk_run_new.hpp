//
//  drawtalk_run_new.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 2/25/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef drawtalk_run_new_hpp
#define drawtalk_run_new_hpp

#include "drawtalk_run.hpp"
#include "scripting.hpp"

namespace dt {

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

Run_Result run(dt::DrawTalk* dt_ctx);





struct Run_Args {
    u8 tmp = 0;
};

}

#endif /* drawtalk_run_new_hpp */
