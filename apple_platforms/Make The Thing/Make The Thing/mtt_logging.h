//
//  mtt_logging.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/17/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef mtt_logging_h
#define mtt_logging_h

#include <os/log.h>
#include <os/signpost.h>


#define MTT_LOGGING_ENABLED (false)




struct MTT_Logger {
    os_log_t logger;
    void* buf;
};

#ifdef __cplusplus
extern "C" {
#endif


void MTT_Logger_init(struct MTT_Logger* logger_out, cstring subsystem, cstring category);
void MTT_Logger_destroy(struct MTT_Logger* logger);

struct MTT_Logger* MTT_active_logger(void);
void MTT_set_active_logger(struct MTT_Logger* logger);
void MTT_set_default_active_logger(void);

void debug_mode_log_enable(bool state);
bool debug_mode_log_is_enabled(void);

#ifdef __cplusplus
}
#endif

#if MTT_LOGGING_ENABLED
#define MTT_log(...) if ((debug_mode_log_is_enabled()) { printf(__VA_ARGS__); }
#define MTT_log_error(...) if ((debug_mode_log_is_enabled()) { printf(__VA_ARGS__); }
#endif

#define mtt_log_public "{public}"

#ifdef NDEBUG

#define MTT_print(...)
#define MTT_print_s(...)
#define MTT_println_s(...)
#define MTT_error(...)

#define MTT_print_begin()
#define MTT_print_end(...)

#define MTT_log_info(...)
#define MTT_log_debug(...)
#define MTT_log_fault(...)
#define MTT_log_error(...)

#define MTT_log_infol(logger, ...)
#define MTT_log_debugl(logger, ...)
#define MTT_log_faultl(logger, ...)
#define MTT_log_errorl(logger, ...)

#define MTT_print_begin()
#define MTT_print_end(...)

#define MTT_log_debug_begin()
#define MTT_log_debug_append(msg)
#define MTT_log_debug_end()
#define MTT_log_info_begin()
#define MTT_log_info_append(msg)
#define MTT_log_info_end()
#define MTT_log_fault_begin()
#define MTT_log_fault_append(msg)
#define MTT_log_fault_end()
#define MTT_log_error_begin()
#define MTT_log_error_append(msg)
#define MTT_log_error_end()

#define MTT_log_debug_beginl(logger)
#define MTT_log_debug_appendl(logger, msg)
#define MTT_log_debug_endl(logger)
#define MTT_log_info_beginl(logger)
#define MTT_log_info_appendl(logger, msg)
#define MTT_log_info_endl(logger)
#define MTT_log_fault_beginl(logger)
#define MTT_log_fault_appendl(logger, msg)
#define MTT_log_fault_endl(logger)
#define MTT_log_error_beginl(logger)
#define MTT_log_error_appendl(logger, msg)
#define MTT_log_error_endl(logger)

#if defined(__OBJC__)
# define MTT_NSLog(...)
#endif

#else

#define MTT_log_debug(format_, ...) if (debug_mode_log_is_enabled()) { os_log_with_type(MTT_active_logger()->logger, OS_LOG_TYPE_DEBUG, format_, __VA_ARGS__); }
#define MTT_log_info(format_, ...) if (debug_mode_log_is_enabled()) { os_log_with_type(MTT_active_logger()->logger, OS_LOG_TYPE_INFO, format_, __VA_ARGS__); }
#define MTT_log_fault(format_, ...) if (debug_mode_log_is_enabled()) { os_log_with_type(MTT_active_logger()->logger, OS_LOG_TYPE_FAULT, format_, __VA_ARGS__); }
#define MTT_log_error(format_, ...) if (debug_mode_log_is_enabled()) { os_log_with_type(MTT_active_logger()->logger, OS_LOG_TYPE_ERROR, format_, __VA_ARGS__); }

#define MTT_print(format_, ...) if (debug_mode_log_is_enabled()) { fprintf(stdout, format_, __VA_ARGS__); }
#define MTT_print_s(...) if (debug_mode_log_is_enabled()) { fprintf(stdout, "%s", __VA_ARGS__); }
#define MTT_println_s(...) if (debug_mode_log_is_enabled()) { fprintf(stdout, "%s\n", __VA_ARGS__); }
#define MTT_error(format_, ...) if (debug_mode_log_is_enabled()) { fprintf(stderr, format_, __VA_ARGS__); }


#define MTT_log_debugl(logger_, format_, ...) if (debug_mode_log_is_enabled()) { os_log_with_type((logger_)->logger, OS_LOG_TYPE_DEBUG, format_, __VA_ARGS__); }
#define MTT_log_infol(logger_, format_, ...) if (debug_mode_log_is_enabled()) { os_log_with_type((logger_)->logger, OS_LOG_TYPE_INFO, format_, __VA_ARGS__); }
#define MTT_log_faultl(logger_, format_, ...) if (debug_mode_log_is_enabled()) { os_log_with_type((logger_)->logger, OS_LOG_TYPE_ERROR, format_, __VA_ARGS__); }
#define MTT_log_errorl(logger_, format_, ...) if (debug_mode_log_is_enabled()) { os_log_with_type((logger_)->logger, OS_LOG_TYPE_ERROR, format_, __VA_ARGS__); }


//
#define MTT_log_info_begin() if (debug_mode_log_is_enabled()) { ((mtt::String*)MTT_active_logger()->buf)->clear(); }
#define MTT_log_info_append(msg_) if (debug_mode_log_is_enabled()) { ((mtt::String*)(MTT_active_logger()->buf))->append(msg_); }
#define MTT_log_info_end(logger_) if (debug_mode_log_is_enabled()) { os_log_with_type(MTT_active_logger()->logger, OS_LOG_TYPE_INFO, "%{public}s", ((mtt::String*) (MTT_active_logger()->buf)  )->c_str()); }

#define MTT_log_debug_begin() if (debug_mode_log_is_enabled()) { ((mtt::String*)MTT_active_logger()->buf)->clear(); }
#define MTT_log_debug_append(msg_) if (debug_mode_log_is_enabled()) { ((mtt::String*)(MTT_active_logger()->buf))->append(msg_); }
#define MTT_log_debug_end() if (debug_mode_log_is_enabled()) { os_log_with_type(MTT_active_logger()->logger, OS_LOG_TYPE_DEBUG, "%{public}s", ((mtt::String*) (MTT_active_logger()->buf)  )->c_str()); }

#define MTT_log_fault_begin() if (debug_mode_log_is_enabled()) { ((mtt::String*)MTT_active_logger()->buf)->clear(); }
#define MTT_log_fault_append(msg_) if (debug_mode_log_is_enabled()) { ((mtt::String*)(MTT_active_logger()->buf))->append(msg_); }
#define MTT_log_fault_end() if (debug_mode_log_is_enabled()) { os_log_with_type(MTT_active_logger()->logger, OS_LOG_TYPE_FAULT, "%{public}s", ((mtt::String*)MTT_active_logger()->buf )->c_str()); }

#define MTT_log_error_begin() if (debug_mode_log_is_enabled()) { ((mtt::String*)MTT_active_logger()->buf)->clear(); }
#define MTT_log_error_append(msg_) if (debug_mode_log_is_enabled()) { ((mtt::String*)(MTT_active_logger()->buf))->append(msg_); }
#define MTT_log_error_end() if (debug_mode_log_is_enabled()) { os_log_with_type(MTT_active_logger()->logger, OS_LOG_TYPE_ERROR, "%{public}s", ((mtt::String*)MTT_active_logger()->logger->buf )->c_str()); }

//
#define MTT_log_info_beginl(logger_) if (debug_mode_log_is_enabled()) { ((mtt::String*)(logger_)->buf)->clear(); }
#define MTT_log_info_appendl(logger_, msg_) if (debug_mode_log_is_enabled()) { ((mtt::String*)((logger_)->buf))->append(msg_); }
#define MTT_log_info_endl(logger_) if (debug_mode_log_is_enabled()) { os_log_with_type((logger_)->logger, OS_LOG_TYPE_INFO, "%{public}s", ((mtt::String*) (logger_->buf)  )->c_str()); }

#define MTT_log_debug_beginl(logger_) if (debug_mode_log_is_enabled()) { ((mtt::String*)(logger_)->buf)->clear(); }
#define MTT_log_debug_appendl(logger_, msg_) if (debug_mode_log_is_enabled()) { ((mtt::String*)((logger_)->buf))->append(msg_); }
#define MTT_log_debug_endl(logger_) if (debug_mode_log_is_enabled()) { os_log_with_type((logger_)->logger, OS_LOG_TYPE_DEBUG, "%{public}s", ((mtt::String*) (logger_->buf)  )->c_str()); }

#define MTT_log_fault_beginl(logger_) if (debug_mode_log_is_enabled()) { ((mtt::String*)(logger_)->buf)->clear(); }
#define MTT_log_fault_appendl(logger_, msg_) if (debug_mode_log_is_enabled()) { ((mtt::String*)((logger_)->buf))->append(msg_); }
#define MTT_log_fault_endl(logger_) if (debug_mode_log_is_enabled()) { os_log_with_type((logger_)->logger, OS_LOG_TYPE_FAULT, "%{public}s", ((mtt::String*)(logger_)->buf )->c_str()); }

#define MTT_log_error_beginl(logger_) if (debug_mode_log_is_enabled()) { ((mtt::String*)(logger_)->buf)->clear(); }
#define MTT_log_error_appendl(logger_, msg_) if (debug_mode_log_is_enabled()) { ((mtt::String*)((logger_)->buf))->append(msg_); }
#define MTT_log_error_endl(logger_) if (debug_mode_log_is_enabled()) { os_log_with_type((logger_)->logger, OS_LOG_TYPE_ERROR, "%{public}s", ((mtt::String*)(logger_)->buf )->c_str()); }

#if defined(__OBJC__)
# define MTT_NSLog(format_, ...) do { NSLog(format_, __VA_ARGS__); } while (0)
#endif

#endif

#define LOG_COLOR_RESET   "\033[0m"
#define LOG_COLOR_BLACK   "\033[30m"      /* Black */
#define LOG_COLOR_RED     "\033[31m"      /* Red */
#define LOG_COLOR_GREEN   "\033[32m"      /* Green */
#define LOG_COLOR_YELLOW  "\033[33m"      /* Yellow */
#define LOG_COLOR_BLUE    "\033[34m"      /* Blue */
#define LOG_COLOR_MAGENTA "\033[35m"      /* Magenta */
#define LOG_COLOR_CYAN    "\033[36m"      /* Cyan */
#define LOG_COLOR_WHITE   "\033[37m"      /* White */
#define LOG_COLOR_BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define LOG_COLOR_BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define LOG_COLOR_BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define LOG_COLOR_BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define LOG_COLOR_BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define LOG_COLOR_BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define LOG_COLOR_BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define LOG_COLOR_BOLDWHITE   "\033[1m\033[37m"      /* Bold White */





#endif /* mtt_logging_h */
