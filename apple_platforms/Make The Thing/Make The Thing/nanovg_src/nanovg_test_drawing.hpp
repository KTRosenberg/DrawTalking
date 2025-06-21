//
//  nanovg_test_drawing.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/6/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef nanovg_test_drawing_hpp
#define nanovg_test_drawing_hpp

//#include "nanovg.h"

void drawColorwheel(NVGcontext* vg, float x, float y, float w, float h, float t);

void drawButton(NVGcontext* vg, int preicon, const char* text, float x, float y, float w, float h, NVGcolor col);

void drawLabel(NVGcontext* vg, const char* text, float x, float y, float w, float h);

void drawParagraph(NVGcontext* vg, float x, float y, float width, float height, float mx, float my);

#endif /* nanovg_test_drawing_hpp */
