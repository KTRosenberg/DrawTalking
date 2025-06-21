#include "c_common.h"

#include <os/log.h>

#ifdef __cplusplus
extern "C" {
#endif
    
    isize gb_fprintf_va(char const *fmt, va_list va)
    {
        static char buf[4096];
        isize len = vsnprintf(buf, sizeof(buf), fmt, va);
        
        //    if (do_log_writes) {
        //        if (!write_file_mode(log_path__, "a", buf, len)) {
        //            fprintf(stderr, "error log write failed\n");
        //            return 0;
        //        }
        //    } else {
        os_log_fault(OS_LOG_DEFAULT, "%{public}s", buf);
        //}
        
        return len;
    }
    
#ifdef __cplusplus
}
#endif
