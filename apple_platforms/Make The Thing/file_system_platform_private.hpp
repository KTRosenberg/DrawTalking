//
//  file_system_platform_private.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/25/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef file_system_platform_private_hpp
#define file_system_platform_private_hpp

namespace mtt {

bool Text_File_load_async_platform_with_block(Text_File_Load_Descriptor* desc, bool (^callback)(Text_File_Load_Result, bool));

}

#endif /* file_system_platform_private_hpp */
