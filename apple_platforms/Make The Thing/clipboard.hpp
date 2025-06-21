//
//  clipboard.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/23/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef clipboard_hpp
#define clipboard_hpp

void Clipboard_paste(const mtt::String& string);
mtt::String Clipboard_copy(void);

#endif /* clipboard_hpp */
