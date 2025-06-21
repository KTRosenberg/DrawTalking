//
//  clipboard.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/23/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "clipboard_platform.hpp"

void Clipboard_paste(const mtt::String& string)
{
    Clipboard_paste_platform(string);
}
mtt::String Clipboard_copy(void)
{
    return Clipboard_copy_platform();
}
