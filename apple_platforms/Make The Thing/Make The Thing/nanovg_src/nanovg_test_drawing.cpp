//
//  nanovg_test_drawing.c
//  Make The Thing
//
//  Created by Toby Rosenberg on 12/6/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "nanovg_test_drawing.hpp"

///

#define ICON_SEARCH 0x1F50D
#define ICON_CIRCLED_CROSS 0x2716
#define ICON_CHEVRON_RIGHT 0xE75E
#define ICON_CHECK 0x2713
#define ICON_LOGIN 0xE740
#define ICON_TRASH 0xE729

void drawColorwheel(NVGcontext* vg, float x, float y, float w, float h, float t)
{
    int i;
    float r0, r1, ax,ay, bx,by, cx,cy, aeps, r;
    float hue = sinf(t * 0.12f);
    NVGpaint paint;
    
    nvgSave(vg);
    
    /*    nvgBeginPath(vg);
     nvgRect(vg, x,y,w,h);
     nvgFillColor(vg, nvgRGBA(255,0,0,128));
     nvgFill(vg);*/
    
    cx = x + w*0.5f;
    cy = y + h*0.5f;
    r1 = (w < h ? w : h) * 0.5f - 5.0f;
    r0 = r1 - 20.0f;
    aeps = 0.5f / r1;    // half a pixel arc length in radians (2pi cancels out).
    
    for (i = 0; i < 6; i++) {
        float a0 = (float)i / 6.0f * NVG_PI * 2.0f - aeps;
        float a1 = (float)(i+1.0f) / 6.0f * NVG_PI * 2.0f + aeps;
        nvgBeginPath(vg);
        nvgArc(vg, cx,cy, r0, a0, a1, NVG_CW);
        nvgArc(vg, cx,cy, r1, a1, a0, NVG_CCW);
        nvgClosePath(vg);
        ax = cx + cosf(a0) * (r0+r1)*0.5f;
        ay = cy + sinf(a0) * (r0+r1)*0.5f;
        bx = cx + cosf(a1) * (r0+r1)*0.5f;
        by = cy + sinf(a1) * (r0+r1)*0.5f;
        paint = nvgLinearGradient(vg, ax,ay, bx,by, nvgHSLA(a0/(NVG_PI*2),1.0f,0.55f,255), nvgHSLA(a1/(NVG_PI*2),1.0f,0.55f,255));
        nvgFillPaint(vg, paint);
        nvgFill(vg);
    }
    
    nvgBeginPath(vg);
    nvgCircle(vg, cx,cy, r0-0.5f);
    nvgCircle(vg, cx,cy, r1+0.5f);
    nvgStrokeColor(vg, nvgRGBA(0,0,0,64));
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);
    
    // Selector
    nvgSave(vg);
    nvgTranslate(vg, cx,cy);
    nvgRotate(vg, hue*NVG_PI*2);
    
    // Marker on
    nvgStrokeWidth(vg, 2.0f);
    nvgBeginPath(vg);
    nvgRect(vg, r0-1,-3,r1-r0+2,6);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,192));
    nvgStroke(vg);
    
    paint = nvgBoxGradient(vg, r0-3,-5,r1-r0+6,10, 2,4, nvgRGBA(0,0,0,128), nvgRGBA(0,0,0,0));
    nvgBeginPath(vg);
    nvgRect(vg, r0-2-10,-4-10,r1-r0+4+20,8+20);
    nvgRect(vg, r0-2,-4,r1-r0+4,8);
    nvgPathWinding(vg, NVG_HOLE);
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    
    // Center triangle
    r = r0 - 6;
    ax = cosf(120.0f/180.0f*NVG_PI) * r;
    ay = sinf(120.0f/180.0f*NVG_PI) * r;
    bx = cosf(-120.0f/180.0f*NVG_PI) * r;
    by = sinf(-120.0f/180.0f*NVG_PI) * r;
    nvgBeginPath(vg);
    nvgMoveTo(vg, r,0);
    nvgLineTo(vg, ax,ay);
    nvgLineTo(vg, bx,by);
    nvgClosePath(vg);
    paint = nvgLinearGradient(vg, r,0, ax,ay, nvgHSLA(hue,1.0f,0.5f,255), nvgRGBA(255,255,255,255));
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    paint = nvgLinearGradient(vg, (r+ax)*0.5f,(0+ay)*0.5f, bx,by, nvgRGBA(0,0,0,0), nvgRGBA(0,0,0,255));
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(0,0,0,64));
    nvgStroke(vg);
    
    // Select circle on triangle
    ax = cosf(120.0f/180.0f*NVG_PI) * r*0.3f;
    ay = sinf(120.0f/180.0f*NVG_PI) * r*0.4f;
    nvgStrokeWidth(vg, 2.0f);
    nvgBeginPath(vg);
    nvgCircle(vg, ax,ay,5);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,192));
    nvgStroke(vg);
    
    paint = nvgRadialGradient(vg, ax,ay, 7,9, nvgRGBA(0,0,0,64), nvgRGBA(0,0,0,0));
    nvgBeginPath(vg);
    nvgRect(vg, ax-20,ay-20,40,40);
    nvgCircle(vg, ax,ay,7);
    nvgPathWinding(vg, NVG_HOLE);
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    
    nvgRestore(vg);
    
    nvgRestore(vg);
}

static char* cpToUTF8(int cp, char* str)
{
    int n = 0;
    if (cp < 0x80) n = 1;
    else if (cp < 0x800) n = 2;
    else if (cp < 0x10000) n = 3;
    else if (cp < 0x200000) n = 4;
    else if (cp < 0x4000000) n = 5;
    else if (cp <= 0x7fffffff) n = 6;
    str[n] = '\0';
    switch (n) {
    case 6: str[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
    case 5: str[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
    case 4: str[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
    case 3: str[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
    case 2: str[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
    case 1: str[0] = cp;
    }
    return str;
}

// Returns 1 if col.rgba is 0.0f,0.0f,0.0f,0.0f, 0 otherwise
int isBlack(NVGcolor col)
{
    if( col.r == 0.0f && col.g == 0.0f && col.b == 0.0f && col.a == 0.0f )
    {
        return 1;
    }
    return 0;
}

void drawButton(NVGcontext* vg, int preicon, const char* text, float x, float y, float w, float h, NVGcolor col)
{
    NVGpaint bg;
    char icon[8];
    float cornerRadius = 4.0f;
    float tw = 0, iw = 0;
    
    bg = nvgLinearGradient(vg, x,y,x,y+h, nvgRGBA(255,255,255,isBlack(col)?16:32), nvgRGBA(0,0,0,isBlack(col)?16:32));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x+1,y+1, w-2,h-2, cornerRadius-1);
    if (!isBlack(col)) {
        nvgFillColor(vg, col);
        nvgFill(vg);
    }
    nvgFillPaint(vg, bg);
    nvgFill(vg);
    
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x+0.5f,y+0.5f, w-1,h-1, cornerRadius-0.5f);
    nvgStrokeColor(vg, nvgRGBA(0,0,0,48));
    nvgStroke(vg);
    
    nvgFontSize(vg, 17.0f);
    nvgFontFace(vg, "sans-bold");
    tw = nvgTextBounds(vg, 0,0, text, NULL, NULL);
    if (preicon != 0) {
        nvgFontSize(vg, h*1.3f);
        nvgFontFace(vg, "icons");
        iw = nvgTextBounds(vg, 0,0, cpToUTF8(preicon,icon), NULL, NULL);
        iw += h*0.15f;
    }
    
    if (preicon != 0) {
        nvgFontSize(vg, h*1.3f);
        nvgFontFace(vg, "icons");
        nvgFillColor(vg, nvgRGBA(255,255,255,96));
        nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
        nvgText(vg, x+w*0.5f-tw*0.5f-iw*0.75f, y+h*0.5f, cpToUTF8(preicon,icon), NULL);
    }
    
    nvgFontSize(vg, 17.0f);
    nvgFontFace(vg, "sans-bold");
    nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, nvgRGBA(0,0,0,160));
    nvgText(vg, x+w*0.5f-tw*0.5f+iw*0.25f,y+h*0.5f-1,text, NULL);
    nvgFillColor(vg, nvgRGBA(255,255,255,160));
    nvgText(vg, x+w*0.5f-tw*0.5f+iw*0.25f,y+h*0.5f,text, NULL);
}

void drawLabel(NVGcontext* vg, const char* text, float x, float y, float w, float h)
{
    NVG_NOTUSED(w);
    
    nvgFontSize(vg, 15.0f);
    nvgFontFace(vg, "sans");
    nvgFillColor(vg, nvgRGBA(255,255,255,128));
    
    nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
    nvgText(vg, x,y+h*0.5f,text, NULL);
}

void drawParagraph(NVGcontext* vg, float x, float y, float width, float height, float mx, float my)
{
    NVGtextRow rows[3];
    NVGglyphPosition glyphs[100];
    std::string text_cpp = "This is a longer chunk of text.\n  \n  Would have used lorem ipsum but she was busy jumping over the lazy dog with the fox and all the men who came to the aid of the party.🎉\U0001F389[" + std::to_string(x) + "]";
    const char* text = text_cpp.c_str();
    
    const char* start;
    const char* end;
    int nrows, i, nglyphs, j, lnum = 0;
    float lineh;
    float caretx, px;
    float bounds[4];
    float a;
    const char* hoverText = "Hover your mouse over the text to see calculated caret position.";
    float gx=0,gy=0;
    int gutter = 0;
    const char* boxText = "Testing\nsome multiline\ntext.";
    NVG_NOTUSED(height);
    
    nvgSave(vg);
    
    nvgFontSize(vg, 16.0f);
    nvgFontFace(vg, "sans");
    nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
    nvgTextMetrics(vg, NULL, NULL, &lineh);
    
    // The text break API can be used to fill a large buffer of rows,
    // or to iterate over the text just few lines (or just one) at a time.
    // The "next" variable of the last returned item tells where to continue.
    start = text;
    end = text + strlen(text);
    usize row = 0;
    while ((nrows = nvgTextBreakLines(vg, start, end, width, rows, 1))) {
        for (i = 0; i < nrows; i++) {
            NVGtextRow* row = &rows[i];
            int hit = mx > x && mx < (x+width) && my >= y && my < (y+lineh);
            
            nvgBeginPath(vg);
            nvgFillColor(vg, nvgRGBA(255,255,255,hit?64:0));
            nvgRect(vg, x + row->minx, y, row->maxx - row->minx, lineh);
            nvgFill(vg);
            
            nvgFillColor(vg, nvgRGBA(255,255,255,255));
//            if (row->start + 4 < row->end) {
//                usize nglyphs = nvgTextGlyphPositions(vg, x, y, row->start, row->end, glyphs, 100);
//
//                nvgFillColor(vg, nvgRGBA(0,0,255,255));
//                nvgText(vg, x, y, row->start, row->start + 2);
//
//                nvgFillColor(vg, nvgRGBA(255,0,0,255));
//                nvgText(vg, glyphs[2].x, y, row->start + 2, row->start + 4);
//
//                nvgFillColor(vg, nvgRGBA(0,0,255,255));
//                nvgText(vg, glyphs[4].x, y, row->start + 4, row->end);
//            } else {
                nvgText(vg, x, y, row->start, row->end);
            //}
            
            
            if (hit) {
                caretx = (mx < x+row->width/2) ? x : x+row->width;
                px = x;
                nglyphs = nvgTextGlyphPositions(vg, x, y, row->start, row->end, glyphs, 100);
                for (j = 0; j < nglyphs; j++) {
                    float x0 = glyphs[j].x;
                    float x1 = (j+1 < nglyphs) ? glyphs[j+1].x : x+row->width;
                    float gx = x0 * 0.3f + x1 * 0.7f;
                    if (mx >= px && mx < gx)
                        caretx = glyphs[j].x;
                    px = gx;
                }
                nvgBeginPath(vg);
                nvgFillColor(vg, nvgRGBA(255,192,0,255));
                nvgRect(vg, caretx, y, 1, lineh);
                nvgFill(vg);
                
                gutter = lnum+1;
                gx = x - 10;
                gy = y + lineh/2;
            }
            
            lnum++;
            y += lineh;
        }
        row += 1;
        
//        nvgFontSize(vg, 16.0f + (row * 10));
//        nvgTextMetrics(vg, NULL, NULL, &lineh);
        // Keep going...
        start = rows[nrows-1].next;
        
        
    }
    
//    if (gutter) {
//        char txt[16];
//        snprintf(txt, sizeof(txt), "%d", gutter);
//        nvgFontSize(vg, 12.0f);
//        nvgTextAlign(vg, NVG_ALIGN_RIGHT|NVG_ALIGN_MIDDLE);
//
//        nvgTextBounds(vg, gx,gy, txt, NULL, bounds);
//
//        nvgBeginPath(vg);
//        nvgFillColor(vg, nvgRGBA(255,192,0,255));
//        nvgRoundedRect(vg, (int)bounds[0]-4,(int)bounds[1]-2, (int)(bounds[2]-bounds[0])+8, (int)(bounds[3]-bounds[1])+4, ((int)(bounds[3]-bounds[1])+4)/2-1);
//        nvgFill(vg);
//
//        nvgFillColor(vg, nvgRGBA(32,32,32,255));
//        nvgText(vg, gx,gy, txt, NULL);
//    }
//
//    y += 20.0f;
//
//    nvgFontSize(vg, 11.0f);
//    nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
//    nvgTextLineHeight(vg, 1.2f);
//
//    nvgTextBoxBounds(vg, x,y, 150, hoverText, NULL, bounds);
//
//    // Fade the tooltip out when close to it.
//    gx = m::clamp(mx, bounds[0], bounds[2]) - mx;
//    gy = m::clamp(my, bounds[1], bounds[3]) - my;
//    a = sqrtf(gx*gx + gy*gy) / 30.0f;
//    a = m::clamp(a, 0.0f, 1.0f);
//    nvgGlobalAlpha(vg, a);
//
//    nvgBeginPath(vg);
//    nvgFillColor(vg, nvgRGBA(220,220,220,255));
//    nvgRoundedRect(vg, bounds[0]-2,bounds[1]-2, (int)(bounds[2]-bounds[0])+4, (int)(bounds[3]-bounds[1])+4, 3);
//    px = (int)((bounds[2]+bounds[0])/2);
//    nvgMoveTo(vg, px,bounds[1] - 10);
//    nvgLineTo(vg, px+7,bounds[1]+1);
//    nvgLineTo(vg, px-7,bounds[1]+1);
//    nvgFill(vg);
//
//    nvgFillColor(vg, nvgRGBA(0,0,0,220));
//    nvgTextBox(vg, x,y, 150, hoverText, NULL);
    
    nvgRestore(vg);
}
