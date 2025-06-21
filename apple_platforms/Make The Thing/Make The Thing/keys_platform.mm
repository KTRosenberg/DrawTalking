//
//  keys_platformh.m
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/20/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "keys_platform.hpp"

//YourEnumForKeys mapping[] = { .... stuff I wrote above .... }
//[8:46 PM]

extern_link_begin()
#if !TARGET_OS_OSX
UI_KEY_TYPE MTT_key_mapping[0xFF] = {
    [0] = UI_KEY_TYPE_UNKNOWN,
    [UIKeyboardHIDUsageKeyboardA] = UI_KEY_TYPE_A,
    [UIKeyboardHIDUsageKeyboardB] = UI_KEY_TYPE_B,
    [UIKeyboardHIDUsageKeyboardC] = UI_KEY_TYPE_C,
    [UIKeyboardHIDUsageKeyboardD] = UI_KEY_TYPE_D,
    [UIKeyboardHIDUsageKeyboardE] = UI_KEY_TYPE_E,
    [UIKeyboardHIDUsageKeyboardF] = UI_KEY_TYPE_F,
    [UIKeyboardHIDUsageKeyboardG] = UI_KEY_TYPE_G,
    [UIKeyboardHIDUsageKeyboardH] = UI_KEY_TYPE_H,
    [UIKeyboardHIDUsageKeyboardI] = UI_KEY_TYPE_I,
    [UIKeyboardHIDUsageKeyboardJ] = UI_KEY_TYPE_J,
    [UIKeyboardHIDUsageKeyboardK] = UI_KEY_TYPE_K,
    [UIKeyboardHIDUsageKeyboardL] = UI_KEY_TYPE_L,
    [UIKeyboardHIDUsageKeyboardM] = UI_KEY_TYPE_M,
    [UIKeyboardHIDUsageKeyboardN] = UI_KEY_TYPE_N,
    [UIKeyboardHIDUsageKeyboardO] = UI_KEY_TYPE_O,
    [UIKeyboardHIDUsageKeyboardP] = UI_KEY_TYPE_P,
    [UIKeyboardHIDUsageKeyboardQ] = UI_KEY_TYPE_Q,
    [UIKeyboardHIDUsageKeyboardR] = UI_KEY_TYPE_R,
    [UIKeyboardHIDUsageKeyboardS] = UI_KEY_TYPE_S,
    [UIKeyboardHIDUsageKeyboardT] = UI_KEY_TYPE_T,
    [UIKeyboardHIDUsageKeyboardU] = UI_KEY_TYPE_U,
    [UIKeyboardHIDUsageKeyboardV] = UI_KEY_TYPE_V,
    [UIKeyboardHIDUsageKeyboardW] = UI_KEY_TYPE_W,
    [UIKeyboardHIDUsageKeyboardX] = UI_KEY_TYPE_X,
    [UIKeyboardHIDUsageKeyboardY] = UI_KEY_TYPE_Y,
    [UIKeyboardHIDUsageKeyboardZ] = UI_KEY_TYPE_Z,
    [UIKeyboardHIDUsageKeyboard1] = UI_KEY_TYPE_1,
    [UIKeyboardHIDUsageKeyboard2] = UI_KEY_TYPE_2,
    [UIKeyboardHIDUsageKeyboard3] = UI_KEY_TYPE_3,
    [UIKeyboardHIDUsageKeyboard4] = UI_KEY_TYPE_4,
    [UIKeyboardHIDUsageKeyboard5] = UI_KEY_TYPE_5,
    [UIKeyboardHIDUsageKeyboard6] = UI_KEY_TYPE_6,
    [UIKeyboardHIDUsageKeyboard7] = UI_KEY_TYPE_7,
    [UIKeyboardHIDUsageKeyboard8] = UI_KEY_TYPE_8,
    [UIKeyboardHIDUsageKeyboard9] = UI_KEY_TYPE_9,
    [UIKeyboardHIDUsageKeyboard0] = UI_KEY_TYPE_0,
    [UIKeyboardHIDUsageKeyboardHyphen]  = UI_KEY_TYPE_HYPHEN,
    [UIKeyboardHIDUsageKeyboardEqualSign] = UI_KEY_TYPE_EQUAL_SIGN,
    [UIKeyboardHIDUsageKeyboardComma]        = UI_KEY_TYPE_COMMA,
    [UIKeyboardHIDUsageKeyboardGraveAccentAndTilde] = UI_KEY_TYPE_GRAVE_ACCENT_AND_TILDE,
    [UIKeyboardHIDUsageKeyboardPeriod] = UI_KEY_TYPE_PERIOD,
    [UIKeyboardHIDUsageKeyboardDeleteOrBackspace] = UI_KEY_TYPE_DELETE_OR_BACKSPACE,
    [UIKeyboardHIDUsageKeyboardReturnOrEnter] = UI_KEY_TYPE_RETURN_OR_ENTER,
    [UIKeyboardHIDUsageKeyboardLeftShift]    = UI_KEY_TYPE_LEFT_SHIFT,
    [UIKeyboardHIDUsageKeyboardRightShift]   = UI_KEY_TYPE_RIGHT_SHIFT,
    [UIKeyboardHIDUsageKeyboardLeftControl]  = UI_KEY_TYPE_LEFT_CONTROL,
    [UIKeyboardHIDUsageKeyboardRightControl] = UI_KEY_TYPE_RIGHT_CONTROL,
    [UIKeyboardHIDUsageKeyboardLeftAlt]      = UI_KEY_TYPE_LEFT_ALT,
    [UIKeyboardHIDUsageKeyboardRightAlt]     = UI_KEY_TYPE_RIGHT_ALT,
    [UIKeyboardHIDUsageKeyboardApplication]  = UI_KEY_TYPE_APPLICATION,
    [UIKeyboardHIDUsageKeyboardSpacebar]     = UI_KEY_TYPE_SPACEBAR,
    [UIKeyboardHIDUsageKeyboardUpArrow]      = UI_KEY_TYPE_UP_ARROW,
    [UIKeyboardHIDUsageKeyboardDownArrow]    = UI_KEY_TYPE_DOWN_ARROW,
    [UIKeyboardHIDUsageKeyboardLeftArrow]    = UI_KEY_TYPE_LEFT_ARROW,
    [UIKeyboardHIDUsageKeyboardRightArrow]   = UI_KEY_TYPE_RIGHT_ARROW,
    
    [UIKeyboardHIDUsageKeyboardTab] = UI_KEY_TYPE_TAB,
    [UIKeyboardHIDUsageKeyboardQuote] = UI_KEY_TYPE_APOSTROPHE,
    [UIKeyboardHIDUsageKeyboardBackslash] = UI_KEY_TYPE_BACKSLASH,
    [UIKeyboardHIDUsageKeyboardSlash] = UI_KEY_TYPE_SLASH,
    
    
};
#else

// https://github.com/libsdl-org/SDL/blob/main/src/events/scancodes_darwin.h
UI_KEY_TYPE MTT_key_mapping[0xFFFF] = {
        [0] = UI_KEY_TYPE_A,
        [1] = UI_KEY_TYPE_S,
        [2] = UI_KEY_TYPE_D,
        [3] = UI_KEY_TYPE_F,
        [4] = UI_KEY_TYPE_H,
        [5] = UI_KEY_TYPE_G,
        [6] = UI_KEY_TYPE_Z,
        [7] = UI_KEY_TYPE_X,
        [8] = UI_KEY_TYPE_C,
        [9] = UI_KEY_TYPE_V,
    
        [11] = UI_KEY_TYPE_B,
        [12] = UI_KEY_TYPE_Q,
        [13] = UI_KEY_TYPE_W,
        [14] = UI_KEY_TYPE_E,
        [15] = UI_KEY_TYPE_R,
        [16] = UI_KEY_TYPE_Y,
        [17] = UI_KEY_TYPE_T,
    
        [18] = UI_KEY_TYPE_1,
        [19] = UI_KEY_TYPE_2,
        [20] = UI_KEY_TYPE_3,
        [21] = UI_KEY_TYPE_4,
        [22] = UI_KEY_TYPE_6,
        [23] = UI_KEY_TYPE_5,
    
        [24] = UI_KEY_TYPE_EQUAL_SIGN,
    
        [25] = UI_KEY_TYPE_9,
        [26] = UI_KEY_TYPE_7,
    
        [27] = UI_KEY_TYPE_HYPHEN,
    
        [28] = UI_KEY_TYPE_8,
        [29] = UI_KEY_TYPE_0,
    
        //[30] = UI_KEY_TYPE_(RIGHT_BRACKET),
    
        [31] = UI_KEY_TYPE_O,
        [32] = UI_KEY_TYPE_U,
    
        //[33] = UI_KEY_TYPE_(LEFT_BRACKET),
    
        [34] = UI_KEY_TYPE_I,
        [35] = UI_KEY_TYPE_P,
    
        [36]  = UI_KEY_TYPE_RETURN_OR_ENTER,
    
        [37] = UI_KEY_TYPE_L,
        [38] = UI_KEY_TYPE_J,
    
        [39] = UI_KEY_TYPE_APOSTROPHE,
    
        [40] = UI_KEY_TYPE_K,
    
        //[41] = UI_KEY_TYPE_SEMICOLON,
    
        [42] = UI_KEY_TYPE_BACKSLASH,
    
        [43]    = UI_KEY_TYPE_COMMA,
    
        [44]  = UI_KEY_TYPE_SLASH,
    
        [45]  = UI_KEY_TYPE_N,
        [46] = UI_KEY_TYPE_M,
    
        [47]      = UI_KEY_TYPE_PERIOD,
    
        [48]     = UI_KEY_TYPE_TAB,
    
        [49]  = UI_KEY_TYPE_SPACEBAR,
        [50]     = UI_KEY_TYPE_GRAVE_ACCENT_AND_TILDE,
    
        [51]      = UI_KEY_TYPE_DELETE_OR_BACKSPACE,
        [52]    = UI_KEY_TYPE_RETURN_OR_ENTER,
        //[53]    = UI_KEY_TYPE_ESCAPE,
    
    
    
    [56] = UI_KEY_TYPE_LEFT_SHIFT,
    //[57] = UI_KEY_TYPE_CAPS_LOCK,
    [58] = UI_KEY_TYPE_LEFT_ALT,
    [59] = UI_KEY_TYPE_LEFT_CONTROL,
    [60] = UI_KEY_TYPE_RIGHT_SHIFT,
    [61] = UI_KEY_TYPE_RIGHT_ALT,
    [62] = UI_KEY_TYPE_RIGHT_CONTROL,
    [123] = UI_KEY_TYPE_LEFT_ARROW,
    [124] = UI_KEY_TYPE_RIGHT_ARROW,
    [125] = UI_KEY_TYPE_DOWN_ARROW,
    [126] = UI_KEY_TYPE_UP_ARROW,
};

///*   0 */   SDL_SCANCODE_A,
///*   1 */   SDL_SCANCODE_S,
///*   2 */   SDL_SCANCODE_D,
///*   3 */   SDL_SCANCODE_F,
///*   4 */   SDL_SCANCODE_H,
///*   5 */   SDL_SCANCODE_G,
///*   6 */   SDL_SCANCODE_Z,
///*   7 */   SDL_SCANCODE_X,
///*   8 */   SDL_SCANCODE_C,
///*   9 */   SDL_SCANCODE_V,
///*  10 */   SDL_SCANCODE_NONUSBACKSLASH, /* SDL_SCANCODE_NONUSBACKSLASH on ANSI and JIS keyboards (if this key would exist there), SDL_SCANCODE_GRAVE on ISO. (The USB keyboard driver actually translates these usage codes to different virtual key codes depending on whether the keyboard is ISO/ANSI/JIS. That's why you have to help it identify the keyboard type when you plug in a PC USB keyboard. It's a historical thing - ADB keyboards are wired this way.) */
///*  11 */   SDL_SCANCODE_B,
///*  12 */   SDL_SCANCODE_Q,
///*  13 */   SDL_SCANCODE_W,
///*  14 */   SDL_SCANCODE_E,
///*  15 */   SDL_SCANCODE_R,
///*  16 */   SDL_SCANCODE_Y,
///*  17 */   SDL_SCANCODE_T,
///*  18 */   SDL_SCANCODE_1,
///*  19 */   SDL_SCANCODE_2,
///*  20 */   SDL_SCANCODE_3,
///*  21 */   SDL_SCANCODE_4,
///*  22 */   SDL_SCANCODE_6,
///*  23 */   SDL_SCANCODE_5,
///*  24 */   SDL_SCANCODE_EQUALS,
///*  25 */   SDL_SCANCODE_9,
///*  26 */   SDL_SCANCODE_7,
///*  27 */   SDL_SCANCODE_MINUS,
///*  28 */   SDL_SCANCODE_8,
///*  29 */   SDL_SCANCODE_0,
///*  30 */   SDL_SCANCODE_RIGHTBRACKET,
///*  31 */   SDL_SCANCODE_O,
///*  32 */   SDL_SCANCODE_U,
///*  33 */   SDL_SCANCODE_LEFTBRACKET,
///*  34 */   SDL_SCANCODE_I,
///*  35 */   SDL_SCANCODE_P,
///*  36 */   SDL_SCANCODE_RETURN,
///*  37 */   SDL_SCANCODE_L,
///*  38 */   SDL_SCANCODE_J,
///*  39 */   SDL_SCANCODE_APOSTROPHE,
///*  40 */   SDL_SCANCODE_K,
///*  41 */   SDL_SCANCODE_SEMICOLON,
///*  42 */   SDL_SCANCODE_BACKSLASH,
///*  43 */   SDL_SCANCODE_COMMA,
///*  44 */   SDL_SCANCODE_SLASH,
///*  45 */   SDL_SCANCODE_N,
///*  46 */   SDL_SCANCODE_M,
///*  47 */   SDL_SCANCODE_PERIOD,
///*  48 */   SDL_SCANCODE_TAB,
///*  49 */   SDL_SCANCODE_SPACE,
///*  50 */   SDL_SCANCODE_GRAVE, /* SDL_SCANCODE_GRAVE on ANSI and JIS keyboards, SDL_SCANCODE_NONUSBACKSLASH on ISO (see comment about virtual key code 10 above) */
///*  51 */   SDL_SCANCODE_BACKSPACE,
///*  52 */   SDL_SCANCODE_KP_ENTER, /* keyboard enter on portables */
///*  53 */   SDL_SCANCODE_ESCAPE,
///*  54 */   SDL_SCANCODE_RGUI,
///*  55 */   SDL_SCANCODE_LGUI,
///*  56 */   SDL_SCANCODE_LSHIFT,
///*  57 */   SDL_SCANCODE_CAPSLOCK,
///*  58 */   SDL_SCANCODE_LALT,
///*  59 */   SDL_SCANCODE_LCTRL,
///*  60 */   SDL_SCANCODE_RSHIFT,
///*  61 */   SDL_SCANCODE_RALT,
///*  62 */   SDL_SCANCODE_RCTRL,
///*  63 */   SDL_SCANCODE_RGUI, /* fn on portables, acts as a hardware-level modifier already, so we don't generate events for it, also XK_Meta_R */
///*  64 */   SDL_SCANCODE_F17,
///*  65 */   SDL_SCANCODE_KP_PERIOD,
///*  66 */   SDL_SCANCODE_UNKNOWN, /* unknown (unused?) */
///*  67 */   SDL_SCANCODE_KP_MULTIPLY,
///*  68 */   SDL_SCANCODE_UNKNOWN, /* unknown (unused?) */
///*  69 */   SDL_SCANCODE_KP_PLUS,
///*  70 */   SDL_SCANCODE_UNKNOWN, /* unknown (unused?) */
///*  71 */   SDL_SCANCODE_NUMLOCKCLEAR,
///*  72 */   SDL_SCANCODE_VOLUMEUP,
///*  73 */   SDL_SCANCODE_VOLUMEDOWN,
///*  74 */   SDL_SCANCODE_MUTE,
///*  75 */   SDL_SCANCODE_KP_DIVIDE,
///*  76 */   SDL_SCANCODE_KP_ENTER, /* keypad enter on external keyboards, fn-return on portables */
///*  77 */   SDL_SCANCODE_UNKNOWN, /* unknown (unused?) */
///*  78 */   SDL_SCANCODE_KP_MINUS,
///*  79 */   SDL_SCANCODE_F18,
///*  80 */   SDL_SCANCODE_F19,
///*  81 */   SDL_SCANCODE_KP_EQUALS,
///*  82 */   SDL_SCANCODE_KP_0,
///*  83 */   SDL_SCANCODE_KP_1,
///*  84 */   SDL_SCANCODE_KP_2,
///*  85 */   SDL_SCANCODE_KP_3,
///*  86 */   SDL_SCANCODE_KP_4,
///*  87 */   SDL_SCANCODE_KP_5,
///*  88 */   SDL_SCANCODE_KP_6,
///*  89 */   SDL_SCANCODE_KP_7,
///*  90 */   SDL_SCANCODE_UNKNOWN, /* unknown (unused?) */
///*  91 */   SDL_SCANCODE_KP_8,
///*  92 */   SDL_SCANCODE_KP_9,
///*  93 */   SDL_SCANCODE_INTERNATIONAL3, /* Cosmo_USB2ADB.c says "Yen (JIS)" */
///*  94 */   SDL_SCANCODE_INTERNATIONAL1, /* Cosmo_USB2ADB.c says "Ro (JIS)" */
///*  95 */   SDL_SCANCODE_KP_COMMA, /* Cosmo_USB2ADB.c says ", JIS only" */
///*  96 */   SDL_SCANCODE_F5,
///*  97 */   SDL_SCANCODE_F6,
///*  98 */   SDL_SCANCODE_F7,
///*  99 */   SDL_SCANCODE_F3,
///* 100 */   SDL_SCANCODE_F8,
///* 101 */   SDL_SCANCODE_F9,
///* 102 */   SDL_SCANCODE_LANG2, /* Cosmo_USB2ADB.c says "Eisu" */
///* 103 */   SDL_SCANCODE_F11,
///* 104 */   SDL_SCANCODE_LANG1, /* Cosmo_USB2ADB.c says "Kana" */
///* 105 */   SDL_SCANCODE_PRINTSCREEN, /* On ADB keyboards, this key is labeled "F13/print screen". Problem: USB has different usage codes for these two functions. On Apple USB keyboards, the key is labeled "F13" and sends the F13 usage code (SDL_SCANCODE_F13). I decided to use SDL_SCANCODE_PRINTSCREEN here nevertheless since SDL applications are more likely to assume the presence of a print screen key than an F13 key. */
///* 106 */   SDL_SCANCODE_F16,
///* 107 */   SDL_SCANCODE_SCROLLLOCK, /* F14/scroll lock, see comment about F13/print screen above */
///* 108 */   SDL_SCANCODE_UNKNOWN, /* unknown (unused?) */
///* 109 */   SDL_SCANCODE_F10,
///* 110 */   SDL_SCANCODE_APPLICATION, /* windows contextual menu key, fn-enter on portables */
///* 111 */   SDL_SCANCODE_F12,
///* 112 */   SDL_SCANCODE_UNKNOWN, /* unknown (unused?) */
///* 113 */   SDL_SCANCODE_PAUSE, /* F15/pause, see comment about F13/print screen above */
///* 114 */   SDL_SCANCODE_INSERT, /* the key is actually labeled "help" on Apple keyboards, and works as such in Mac OS, but it sends the "insert" usage code even on Apple USB keyboards */
///* 115 */   SDL_SCANCODE_HOME,
///* 116 */   SDL_SCANCODE_PAGEUP,
///* 117 */   SDL_SCANCODE_DELETE,
///* 118 */   SDL_SCANCODE_F4,
///* 119 */   SDL_SCANCODE_END,
///* 120 */   SDL_SCANCODE_F2,
///* 121 */   SDL_SCANCODE_PAGEDOWN,
///* 122 */   SDL_SCANCODE_F1,
///* 123 */   SDL_SCANCODE_LEFT,
///* 124 */   SDL_SCANCODE_RIGHT,
///* 125 */   SDL_SCANCODE_DOWN,
///* 126 */   SDL_SCANCODE_UP,
///* 127 */   SDL_SCANCODE_POWER

#endif

extern_link_end();


