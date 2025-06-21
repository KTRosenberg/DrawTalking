//
//  text_file_platform.m
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/13/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "file_system_platform.hpp"
#include "file_system_platform_private.hpp"
#include "job_dispatch.hpp"
#include "mtt_core_platform.h"

namespace mtt {

static NSString* main_dir_key = @"main_dir";
static NSString* main_directory_path = nil;

void* File_System_get_main_directory_path_platform(void)
{
    return (__bridge void*)main_directory_path;
}

void File_System_get_main_directory_path_cstring_platform(void (*callback)(cstring str, usize length, void* ctx), void* ctx)
{
    @autoreleasepool {
        cstring str = [main_directory_path  cStringUsingEncoding:NSUTF8StringEncoding];
        callback(str, [main_directory_path lengthOfBytesUsingEncoding:NSUTF8StringEncoding], ctx);
    }
}

#if TARGET_OS_OSX

void save_bookmark_data(NSURL* work_dir)
{
    @autoreleasepool {
        NSError* error = nil;
        NSData* data = [work_dir bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:&error];
        
        if (error != nil) {
            NSLog(@"error creating directory: %@", error);
            return;
        }
        
        [[NSUserDefaults standardUserDefaults] setObject:data forKey:@"main_dir"];
    }
}

NSURL* init_file_access(NSData* bookmark_data)
{
    @autoreleasepool {
        BOOL is_stale = NO;
        NSError* error = nil;
        NSURL* url = [NSURL URLByResolvingBookmarkData:bookmark_data options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&is_stale error:&error];
        if (error != nil) {
            NSLog(@"Error resolving bookmark: %@", error);
            return nil;
        }
        if (is_stale) {
            NSLog(@"bookmark is stale, need to re-save");
            save_bookmark_data(url);
        }
        return url;
    }
}

NSURL* prompt_for_working_directory_permission()
{
    @autoreleasepool {
        NSOpenPanel* open_panel = [NSOpenPanel openPanel];
        open_panel.message = @"Choose working directory";
        open_panel.prompt = @"Choose";
        open_panel.canChooseFiles = NO;
        open_panel.canChooseDirectories = YES;
        open_panel.allowsMultipleSelection = NO;
        
        auto response = [open_panel runModal];
        if (response != NSModalResponseOK) {
            return nil;
        }
    
        return open_panel.URLs.firstObject;
    }
}

NSURL* scoped_working_dir = nil;

NSString* resolve_working_dir(void)
{
    @autoreleasepool {
        NSString* path = nil;
        
        NSData* bookmark_data = [[NSUserDefaults standardUserDefaults] dataForKey:main_dir_key];
        if (bookmark_data == nil) {
            NSURL* working_dir = prompt_for_working_directory_permission();
            if (working_dir == nil) {
                path = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0];
            } else {
                scoped_working_dir = working_dir;
                [scoped_working_dir startAccessingSecurityScopedResource];
                save_bookmark_data(working_dir);
                path = [working_dir path];
            }
        } else {
            NSURL* working_dir = init_file_access(bookmark_data);
            if (working_dir == nil) {
                path = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0];
            } else {
                scoped_working_dir = working_dir;
                [scoped_working_dir startAccessingSecurityScopedResource];
                path = [working_dir path];
            }
        }
        
        return path;
    }
}

#endif

bool is_init = false;

void File_System_deinit_platform(void)
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    if (scoped_working_dir == nil) {
        return;
    }
    
    [scoped_working_dir stopAccessingSecurityScopedResource];
#endif
}

void File_System_init_platform(void)
{
    if (is_init) {
        return;
    }
    
    is_init = true;
    @autoreleasepool {
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    //    NSArray * paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    //    NSString * path = [paths objectAtIndex:0];
        NSString* path = resolve_working_dir();
        
        NSFileManager* file_manager = [NSFileManager defaultManager];
        NSString* directory_path = [path stringByAppendingFormat:@"/%@-data", [NSBundle mainBundle].infoDictionary[@"SHORT_NAME"]];
        
        NSError * error = nil;
        [file_manager createDirectoryAtPath:directory_path
                withIntermediateDirectories:YES
                                 attributes:nil
                                      error:&error];
        if (error != nil) {
            NSLog(@"error creating directory: %@", error);
        } else {
            main_directory_path = directory_path;
        }
#else
        NSFileManager* file_manager = [NSFileManager defaultManager];
        MTT_UNUSED(file_manager);
        
        NSArray * paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString * path = [paths objectAtIndex:0];
        NSString* directory_path = path;
        
        main_directory_path = directory_path;
#endif
    }
}

bool File_System_add_path_platform(cstring in_path)
{
    @autoreleasepool {
    NSFileManager* file_manager = [NSFileManager defaultManager];
    
    NSError * error = nil;
    [file_manager createDirectoryAtPath:[main_directory_path stringByAppendingFormat:@"/%@", [NSString stringWithUTF8String:in_path]]
            withIntermediateDirectories:YES
                             attributes:nil
                                  error:&error];
    if (error != nil) {
        NSLog(@"error creating directory: %@", error);
        return false;
    }
        
    }
    
    return true;
}

bool File_System_remove_path_platform(cstring in_path)
{
    @autoreleasepool {
    NSFileManager* file_manager = [NSFileManager defaultManager];
    
    NSError * error = nil;
    
    [file_manager removeItemAtPath:[main_directory_path stringByAppendingFormat:@"/%@", [NSString stringWithUTF8String:in_path]] error:&error];
    if (error != nil) {
        NSLog(@"error removing directory: %@", error);
//        {
//                            NSArray* dirs = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[main_directory_path      stringByAppendingString:@"/testing"]
//                                                                                                error:NULL];
//            
//            //                [dirs enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
//            //                    NSString *filename = (NSString *)obj;
//            //                    NSString *extension = [[filename pathExtension] lowercaseString];
//            //                    int BP = 0;
//            //                }];
//        }
        return false;
    }
        
    }
    
    return true;
}

void File_System_find_file_paths_with_extension_platform(NSString* basePath, NSArray* fileTypes, std::vector<std::string>& files);

void File_System_find_file_paths_with_extension_platform(cstring relative_root, cstring* file_types, usize file_type_count, std::vector<std::string>& files)
{
    @autoreleasepool {
        NSMutableArray* fileTypes = [[NSMutableArray alloc] initWithCapacity:file_type_count];
        if (fileTypes) {

            for (usize i = 0; i < file_type_count; i += 1) {
                [fileTypes addObject: [NSString stringWithFormat: @"%s", file_types[i]]];
            }
            
            NSString* root = [NSString stringWithFormat: @"%s/", relative_root];
            File_System_find_file_paths_with_extension_platform(root, fileTypes, files);
        }
    }
}
void File_System_find_file_paths_with_extension_platform(NSString* basePath, NSArray* fileTypes, std::vector<std::string>& files) {
    NSFileManager *defFM = [NSFileManager defaultManager];
    NSError *error = nil;
    NSArray *dirPath = [defFM contentsOfDirectoryAtPath:[NSString stringWithFormat: @"%@/%@", main_directory_path, basePath] error:&error];
    for(NSString *path in dirPath){
        BOOL isDir;
        NSString* full_path = [[NSString stringWithFormat: @"%@/%@", main_directory_path, basePath] stringByAppendingPathComponent:path];
        if([defFM fileExistsAtPath:full_path isDirectory:&isDir] && isDir){
            File_System_find_file_paths_with_extension_platform([NSString stringWithFormat: @"%@/%@", basePath, path], fileTypes, files);
        }
    }

    NSArray *mediaFiles = [dirPath pathsMatchingExtensions:fileTypes];
    for(NSString *fileName in mediaFiles) {
        NSString* full_file_name = [[basePath stringByAppendingPathComponent:fileName] stringByDeletingPathExtension];
        files.push_back(std::string([full_file_name UTF8String]));
    }
}

bool File_System_load_async_platform(cstring extension, Text_File_Load_Descriptor* desc, bool (*callback)(Text_File_Load_Descriptor, bool)) {
    //const char *command = [str UTF8String];
    return false;
}

void Text_File_write_platform(Text_File_Write_Descriptor* desc, void (*callback)(bool status))
{

    @autoreleasepool {
        NSFileManager* file_manager = [NSFileManager defaultManager];
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
        NSString* path_string = nil;
        
        if (desc->path.extension.size() > 0) {
            path_string = [main_directory_path stringByAppendingFormat:@"/%@.%@", [NSString stringWithUTF8String:desc->path.path.c_str()], [NSString stringWithUTF8String:desc->path.extension.c_str()]];
        } else {
            path_string = [main_directory_path stringByAppendingFormat:@"/%@", [NSString stringWithUTF8String:desc->path.path.c_str()]];
        }
        
#else
        NSString* path_string = nil;
        {
            
            
            {
                //                NSArray* dirs = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[main_directory_path      stringByAppendingString:@"/images"]
                //                                                                                    error:NULL];
                
                //                [dirs enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                //                    NSString *filename = (NSString *)obj;
                //                    NSString *extension = [[filename pathExtension] lowercaseString];
                //                    int BP = 0;
                //                }];
            }
            
            Path_Info* path = &desc->path;
            if (path->extension.size() > 0) {
                NSString* extension_string = [NSString stringWithUTF8String:path->extension.c_str()];
                path_string = [main_directory_path stringByAppendingFormat:@"/%@.%@", [NSString stringWithUTF8String:path->path.c_str()], extension_string];
                BOOL file_exists = [file_manager fileExistsAtPath:path_string];
                if (!file_exists) {
                    path_string = [main_directory_path stringByAppendingFormat:@"/%@.%@", [NSString stringWithUTF8String:path->path.c_str()], [extension_string localizedUppercaseString]];
                }
            } else {
                path_string = [main_directory_path stringByAppendingFormat:@"/%@", [NSString stringWithUTF8String:path->path.c_str()]];
            }

            //            [file_manager urls]
            //            NSArray<NSURL*>* urls = [file_manager URLsForDirectory: inDomains:NSUserDomainMask];
            //main_directory_path
        }
#endif
        BOOL file_exists = [file_manager fileExistsAtPath:path_string];
        NSURL* url = [NSURL fileURLWithPath:path_string];
        __block auto* callback_saved = callback;
        __block NSURL* url_to_write = url;
        __block NSString* str = [NSString stringWithUTF8String:desc->to_write.c_str()];
        __block auto* custom_data = desc->custom_data;
        if (file_exists && desc->flags == WRITE_FLAG_APPEND) {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
                
                NSError* err = nil;
                NSData* data = [NSData dataWithContentsOfURL:url options:NSDataReadingMappedIfSafe error:&err];
                
                if (err != nil) {
                    __block NSError* error = err;
                    dispatch_async(dispatch_get_main_queue(), ^{
                        NSLog(@"Error %@\n", [error localizedDescription]);
                        callback_saved(false);
                        error = nil;
                    });
                    return;
                }
                
                NSString* contents = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] stringByAppendingString:str];
                
                data = [contents dataUsingEncoding:NSUTF8StringEncoding];
                err = nil;
                [data writeToURL:url_to_write options:NSDataWritingAtomic error:&err];
                if (err) {
                    NSLog(@"Writing to file with path %s failed: %@", desc->path.path.c_str(), err);
                    return;
                } else {
                    int BP = 1;
                }
                //cstring data_string = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] cStringUsingEncoding:NSUTF8StringEncoding];
                
                
                dispatch_sync(dispatch_get_main_queue(), ^{
                    callback_saved(true);
                });
                
            });
            
        } else {

            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
                NSData* data = [str dataUsingEncoding:NSUTF8StringEncoding];
                NSError* err;
                [data writeToURL:url_to_write options:NSDataWritingAtomic error:&err];
                if (err) {
                    NSLog(@"Writing to file with path %s failed: %@", desc->path.path.c_str(), err);
                    return;
                } else {
                    int BP = 1;
                }
            });
        }
        
        
    }
}

bool Text_File_load_async_platform(Text_File_Load_Descriptor* desc, bool (*callback)(Text_File_Load_Result, bool))
{
    @autoreleasepool {
        __block auto captured_callback = callback;
        
        return Text_File_load_async_platform_with_block(desc, ^bool(Text_File_Load_Result result, bool status) {
            return captured_callback(result, status);
        });
    }
}

bool Text_File_load_async_platform_with_block(Text_File_Load_Descriptor* desc, bool (^callback)(Text_File_Load_Result, bool))
{
    @autoreleasepool {
        NSMutableArray* urls = [[NSMutableArray alloc] initWithCapacity:desc->paths.size()];
        NSFileManager* file_manager = [NSFileManager defaultManager];
        
        __block bool (^callback_to_use)(Text_File_Load_Result result, bool status) = callback;
        
        for (auto it = desc->paths.begin(); it != desc->paths.end(); ++it) {
            NSURL* url = nil;
            
            if (it->flags == PATH_FLAG_REMOTE) {
                url = [NSURL URLWithString:[NSString stringWithUTF8String:it->path.c_str()]];
                if (url == nil) {
                    return false;
                }
                it->name = [[[url URLByDeletingPathExtension] lastPathComponent] UTF8String];
                it->extension = [[url pathExtension] UTF8String];
                [urls addObject:url];
                continue;
            }
            
    #if TARGET_OS_OSX || TARGET_OS_MACCATALYST
            {
                NSString* path_string = nil;
                if (it->extension.size() > 0) {
                    path_string = [main_directory_path stringByAppendingFormat:@"/%@.%@", [NSString stringWithUTF8String:it->path.c_str()], [NSString stringWithUTF8String:it->extension.c_str()]];
                } else {
                    path_string = [main_directory_path stringByAppendingFormat:@"/%@", [NSString stringWithUTF8String:it->path.c_str()]];
                }
                BOOL file_exists = [file_manager fileExistsAtPath:path_string];
                if (file_exists) {
                    url = [NSURL fileURLWithPath:path_string];
                    [urls addObject:url];
                    continue;
                }
                //            [file_manager urls]
                //            NSArray<NSURL*>* urls = [file_manager URLsForDirectory: inDomains:NSUserDomainMask];
                //main_directory_path
            }
    #else
            {
                NSFileManager* file_manager = [NSFileManager defaultManager];
                NSString* path_string = nil;
                {
                    //                NSArray* dirs = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[main_directory_path      stringByAppendingString:@"/images"]
                    //                                                                                    error:NULL];
                    
                    //                [dirs enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                    //                    NSString *filename = (NSString *)obj;
                    //                    NSString *extension = [[filename pathExtension] lowercaseString];
                    //                    int BP = 0;
                    //                }];
                }
                if (it->extension.size() > 0) {
                    NSString* extension_string = [NSString stringWithUTF8String:it->extension.c_str()];
                    path_string = [main_directory_path stringByAppendingFormat:@"/%@.%@", [NSString stringWithUTF8String:it->path.c_str()], extension_string];
                    BOOL file_exists = [file_manager fileExistsAtPath:path_string];
                    if (!file_exists) {
                        path_string = [main_directory_path stringByAppendingFormat:@"/%@.%@", [NSString stringWithUTF8String:it->path.c_str()], [extension_string localizedUppercaseString]];
                    }
                } else {
                    path_string = [main_directory_path stringByAppendingFormat:@"/%@", [NSString stringWithUTF8String:it->path.c_str()]];
                }
                BOOL file_exists = [file_manager fileExistsAtPath:path_string];
                if (file_exists) {
                    url = [NSURL fileURLWithPath:path_string];
                    [urls addObject:url];
                    continue;
                }
                //            [file_manager urls]
                //            NSArray<NSURL*>* urls = [file_manager URLsForDirectory: inDomains:NSUserDomainMask];
                //main_directory_path
            }
            
            #endif
            url = [[NSBundle mainBundle] URLForResource:[NSString stringWithUTF8String:it->path.c_str()]
                                          withExtension:[NSString stringWithUTF8String:it->extension.c_str()]];
            
            if (url == nil) {
                callback_to_use({}, false);
                return false;
            }
            
            [urls addObject:url];
        }
        
        
        {
            
            __block Text_File_Load_Result result;
            result.custom_data = desc->custom_data;
            result.text.resize(desc->paths.size());
            
            dispatch_queue_t queue = mtt_core_platform_ctx()->bg_text_io_queue;
            NSEnumerator* e = [urls objectEnumerator];
            NSURL* a_url;
            
            __block bool was_error = false;
            
            __block usize count_completed = 0;
            usize idx = 0;
            while (a_url = [e nextObject]) {
                __block NSURL* url = a_url;
                __block usize which_idx = idx;
                idx += 1;
                dispatch_async(queue, ^{
                    if (was_error) {
                        return;
                    }
                    
                    @autoreleasepool {
                    
                    NSError* err = nil;
                    NSData* data = [NSData dataWithContentsOfURL:url options:NSDataReadingMappedIfSafe error:&err];
                    
                    count_completed += 1;
                    if (err != nil) {
                        was_error = true;
                        __block NSError* error = err;
                        dispatch_async(dispatch_get_main_queue(), ^{
                            NSLog(@"Error %@\n", [error localizedDescription]);
                            callback_to_use({}, false);
                            error = nil;
                        });
                        return;
                    }
                    
                    cstring data_string = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] cStringUsingEncoding:NSUTF8StringEncoding];
                    result.text[which_idx] = data_string;
                    
                    if (count_completed == result.text.size()) {
                        dispatch_sync(dispatch_get_main_queue(), ^{
                            callback_to_use(result, true);
                        });
                    }
                        
                    }
                });
            }
        }
        
    }
    
    
    return true;

}



void File_System_rename(const mtt::String& original_path, const mtt::String& new_path)
{
    
    @autoreleasepool {
        NSString* initPath = [NSString stringWithUTF8String:original_path.c_str()];
        NSString* newPath = [NSString stringWithUTF8String:new_path.c_str()];
        NSLog(@"File_System_rename: from:[%@] to:[%@]", initPath, newPath);
        NSError *error = nil;
        [[NSFileManager defaultManager] moveItemAtPath:initPath toPath:newPath error:&error];
    }
}


void File_System_create_file_platform(cstring rel_path, const File_Args& args)
{
    NSFileManager* file_manager = [NSFileManager defaultManager];
    
    @autoreleasepool {
        NSData *data = [NSData dataWithBytes:args.data length:args.byte_size];
        
        [file_manager createFileAtPath:[main_directory_path stringByAppendingFormat:@"/%@", [NSString stringWithUTF8String:rel_path]]
                                        contents:data
         // https://developer.apple.com/documentation/foundation/nsfileattributekey?language=objc
                                        attributes:nil//(NSDictionary<NSFileAttributeKey, id> *)attr;
         ];
        
    }
}

bool File_System_delete_file_platform(cstring rel_path)
{
    return File_System_remove_path_platform(rel_path);
}
    
}


