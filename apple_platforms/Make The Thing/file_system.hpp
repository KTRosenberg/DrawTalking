//
//  text_file.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/13/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef text_file_hpp
#define text_file_hpp

namespace mtt {

struct Text_File_Load_Result {
    std::vector<std::string> text;
    
    //bool (*handler)(Text_File_Load_Result);
    
    void* custom_data = nullptr;
};

struct Text_File_Write_Result {
    //bool (*handler)(Text_File_Write_Result);
    void* custom_data = nullptr;
};

struct Path_Info {
    mtt::String path;
    mtt::String extension;
    mtt::String name;
    uint64 flags = 0;
};

struct Text_File_Loader;

const uint64 PATH_FLAG_CHECK_ALL     = 0;
const uint64 PATH_FLAG_BUILTIN       = (1 << 0);
const uint64 PATH_FLAG_FILE_SYSTEM   = (1 << 1);
const uint64 PATH_FLAG_SYSTEM_REMOTE = (1 << 2);
const uint64 PATH_FLAG_REMOTE        = (1 << 3);

struct Text_File_Load_Descriptor {
    void* custom_data;
    std::vector<Path_Info> paths;
};

struct Text_File_Write_Descriptor {
    void* custom_data;
    Path_Info path;
    mtt::String to_write;
    uint64 flags = 0;
};


void File_System_init(void);

typedef bool (*Text_File_Load_Proc)(Text_File_Load_Result result, bool status);
bool File_System_add_path(cstring in_path);
bool File_System_remove_path(cstring in_path);

bool Text_File_load_async(Text_File_Load_Descriptor* desc, bool (*callback)(Text_File_Load_Result result, bool status));
void Text_File_write(Text_File_Write_Descriptor* desc, void (*callback)(bool status));

constexpr const uint64 WRITE_FLAG_APPEND = 0x1;

struct File_Args {
    void* data;
    usize byte_size;
};

void File_System_create_file(cstring rel_path, const File_Args& args);
bool File_System_delete_file(cstring rel_path);

void File_System_find_file_paths_with_extension(cstring relative_root, cstring* file_types, usize file_type_count, std::vector<std::string>& files);

void File_System_get_main_directory_path_cstring(void (*callback)(cstring str, usize length, void* ctx), void* ctx);

void File_System_rename(const mtt::String& original_path, const mtt::String& new_path);

}

#endif /* text_file_hpp */
