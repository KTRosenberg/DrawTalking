//
//  text_file_platform.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/13/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef text_file_platform_h
#define text_file_platform_h

#include "file_system.hpp"

namespace mtt {

void* File_System_get_main_directory_path_platform(void);
void File_System_get_main_directory_path_cstring_platform(void (*callback)(cstring str, usize length, void* ctx), void* ctx);

void File_System_init_platform(void);
void File_System_deinit_platform(void);
bool File_System_add_path_platform(cstring in_path);
bool File_System_remove_path_platform(cstring in_path);

bool Text_File_load_async_platform(Text_File_Load_Descriptor* desc, bool (*callback)(Text_File_Load_Result, bool));
void Text_File_write_platform(Text_File_Write_Descriptor* desc, void (*callback)(bool status));

void File_System_create_file_platform(cstring rel_path, const File_Args& args);
bool File_System_delete_file_platform(cstring rel_path);

void File_System_find_file_paths_with_extension_platform(cstring relative_root, cstring* file_types, usize file_type_count, std::vector<std::string>& files);

}


#endif /* text_file_platform_h */
