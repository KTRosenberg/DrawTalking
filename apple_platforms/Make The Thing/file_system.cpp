//
//  text_file.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/13/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "file_system_platform.hpp"

namespace mtt {

void File_System_init(void)
{
    File_System_init_platform();
}

bool File_System_add_path(cstring in_path)
{
    return File_System_add_path_platform(in_path);
}
bool File_System_remove_path(cstring in_path)
{
    return File_System_remove_path_platform(in_path);
}

bool Text_File_load_async(Text_File_Load_Descriptor* desc, bool (*callback)(Text_File_Load_Result result, bool status))
{
    return Text_File_load_async_platform(desc, callback);
}
void Text_File_write(Text_File_Write_Descriptor* desc, void (*callback)(bool status))
{
    return Text_File_write_platform(desc, callback);
}

void File_System_create_file(cstring rel_path, const File_Args& args)
{
    File_System_create_file_platform(rel_path, args);
}
bool File_System_delete_file(cstring rel_path)
{
    return File_System_delete_file_platform(rel_path);
}

void File_System_find_file_paths_with_extension(cstring relative_root, cstring* file_types, usize file_type_count, std::vector<std::string>& files)
{
    return File_System_find_file_paths_with_extension_platform(relative_root, file_types, file_type_count, files);
}

void File_System_get_main_directory_path_cstring(void (*callback)(cstring str, usize length, void* ctx), void* ctx)
{
    File_System_get_main_directory_path_cstring_platform(callback, ctx);
}
                                         
}
