//
//  dynamic_library.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef dynamic_library_hpp
#define dynamic_library_hpp


typedef void* (*PROC_MTT_Dynamic_Library)(void* ctx);

struct MTT_Dynamic_Library {
    mtt::String version_number = "";
    mtt::String name = {};
    void* handle = NULL;
    robin_hood::unordered_node_map<std::string, PROC_MTT_Dynamic_Library> procedures = {};
    
    void* operator()(const std::string& name, void* args);
};

static inline bool MTT_Dynamic_Library_init(MTT_Dynamic_Library* dylib, const std::string& path)
{
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    auto* handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        MTT_log_error("Error loading dynamic library %s, %s\n", path.c_str(), dlerror());
        return false;
    }
    if (dylib->handle != NULL) {
        dlclose(dylib->handle);        
    }
    dylib->procedures.clear();
    dylib->handle = handle;
    return true;
#else
    return false;
#endif
}

static inline void MTT_Dynamic_Library_close(MTT_Dynamic_Library* dylib)
{
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    if (dylib->handle == NULL) {
        return;
    }
    
    dlclose(dylib->handle);
    dylib->procedures.clear();
    dylib->handle = NULL;
#endif
}



static inline bool MTT_Dynamic_Library_load_symbol(MTT_Dynamic_Library* dylib, const std::string& name, void** sym)
{
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    void* s = dlsym(dylib->handle, name.c_str());
    if (s) {
        *sym = s;
        return true;
    }
    
    fprintf(stderr, "Error loading dynamic library symbol %s, %s\n", name.c_str(), dlerror());
    
    return false;
#else
    return false;
#endif
}


static inline bool MTT_Dynamic_Library_load_proc(MTT_Dynamic_Library* dylib, const std::string& name, PROC_MTT_Dynamic_Library* proc)
{
    void** as_typeless = (void**)proc;
    return MTT_Dynamic_Library_load_symbol(dylib, name, as_typeless);
}

static inline void MTT_Dynamic_Library_register_proc(MTT_Dynamic_Library* dylib, const std::string& name, PROC_MTT_Dynamic_Library proc)
{
    dylib->procedures[name] = proc;
}

static inline bool MTT_Dynamic_Library_load_and_register_proc(MTT_Dynamic_Library* dylib, const std::string& name, PROC_MTT_Dynamic_Library* proc)
{
    if (MTT_Dynamic_Library_load_proc(dylib, name, proc)) {
        MTT_Dynamic_Library_register_proc(dylib, name, *proc);
        return true;
    }
    
    return false;
}

static inline void* MTT_Dynamic_Library_call_proc(MTT_Dynamic_Library* dylib, const std::string& name, void* ctx)
{
    auto find = dylib->procedures.find(name);
    if (find != dylib->procedures.end()) {
        return find->second(ctx);
    } else {
        return nullptr;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP

#define DEFINE_MTT_DLL_CALL \
\
[[clang::noinline]] \
void MTT_DLL_CALL(MTT_Dynamic_Library* dylib, cstring name, void* ctx) \
{ \
    MTT_Dynamic_Library_call_proc(dylib, std::string(name), ctx); \
} \

#else

#define DEFINE_MTT_DLL_CALL \
inline void MTT_DLL_CALL(MTT_Dynamic_Library* dylib, cstring name, void* ctx) \
{\
   \
}\

#endif

#ifdef __cplusplus
}
#endif


#endif /* dynamic_library_hpp */
