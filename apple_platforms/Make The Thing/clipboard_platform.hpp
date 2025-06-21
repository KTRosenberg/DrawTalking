//
//  clipboard_platform.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/23/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef clipboard_platform_hpp
#define clipboard_platform_hpp

#include "clipboard.hpp"

void Clipboard_paste_platform(const mtt::String& string);
mtt::String Clipboard_copy_platform(void);


#endif /* clipboard_platform_hpp */
