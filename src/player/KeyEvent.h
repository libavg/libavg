//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#ifndef _KeyEvent_h_
#define _KeyEvent_h_

#include "Event.h"

#include <string>

namespace avg {

class KeyEvent : public Event {
    public:
        KeyEvent(Type eventType, unsigned char scanCode, int keyCode, 
                const std::string& keyString, int modifiers);
        virtual ~KeyEvent();

        unsigned char getScanCode() const;
        int getKeyCode() const;
        const std::string& getKeyString() const;
        int getModifiers() const;

        void trace();

    private: 
        int m_ScanCode; 
        int m_KeyCode;
        std::string m_KeyString;
        int m_Modifiers;
};

    namespace key {
        // Plattform independent key codes and modifiers follow. 
        // The constants are borrowed from SDL.

        // Enumeration of valid key mods (possibly OR'd together).
        const long KEYMOD_NONE  = 0x0000;
        const long KEYMOD_LSHIFT= 0x0001;
        const long KEYMOD_RSHIFT= 0x0002;
        const long KEYMOD_LCTRL = 0x0040;
        const long KEYMOD_RCTRL = 0x0080;
        const long KEYMOD_LALT  = 0x0100;
        const long KEYMOD_RALT  = 0x0200;
        const long KEYMOD_LMETA = 0x0400;
        const long KEYMOD_RMETA = 0x0800;
        const long KEYMOD_NUM   = 0x1000;
        const long KEYMOD_CAPS  = 0x2000;
        const long KEYMOD_MODE  = 0x4000;
        const long KEYMOD_RESERVED = 0x8000;

        const long KEYMOD_CTRL    = (KEYMOD_LCTRL|KEYMOD_RCTRL);
        const long KEYMOD_SHIFT   = (KEYMOD_LSHIFT|KEYMOD_RSHIFT);
        const long KEYMOD_ALT     = (KEYMOD_LALT|KEYMOD_RALT);
        const long KEYMOD_META    = (KEYMOD_LMETA|KEYMOD_RMETA); 


        // Key syms. 'cleverly chosen to map to ASCII', sais SDL.
        const long KEY_UNKNOWN     = 0;
        const long KEY_FIRST       = 0;
        const long KEY_BACKSPACE       = 8;
        const long KEY_TAB     = 9;
        const long KEY_CLEAR       = 12;
        const long KEY_RETURN      = 13;
        const long KEY_PAUSE       = 19;
        const long KEY_ESCAPE      = 27;
        const long KEY_SPACE       = 32;
        const long KEY_EXCLAIM     = 33;
        const long KEY_QUOTEDBL        = 34;
        const long KEY_HASH        = 35;
        const long KEY_DOLLAR      = 36;
        const long KEY_AMPERSAND       = 38;
        const long KEY_QUOTE       = 39;
        const long KEY_LEFTPAREN       = 40;
        const long KEY_RIGHTPAREN      = 41;
        const long KEY_ASTERISK        = 42;
        const long KEY_PLUS        = 43;
        const long KEY_COMMA       = 44;
        const long KEY_MINUS       = 45;
        const long KEY_PERIOD      = 46;
        const long KEY_SLASH       = 47;
        const long KEY_0           = 48;
        const long KEY_1           = 49;
        const long KEY_2           = 50;
        const long KEY_3           = 51;
        const long KEY_4           = 52;
        const long KEY_5           = 53;
        const long KEY_6           = 54;
        const long KEY_7           = 55;
        const long KEY_8           = 56;
        const long KEY_9           = 57;
        const long KEY_COLON       = 58;
        const long KEY_SEMICOLON       = 59;
        const long KEY_LESS        = 60;
        const long KEY_EQUALS      = 61;
        const long KEY_GREATER     = 62;
        const long KEY_QUESTION        = 63;
        const long KEY_AT          = 64;
        // Skip uppercase letters
        const long KEY_LEFTBRACKET = 91;
        const long KEY_BACKSLASH       = 92;
        const long KEY_RIGHTBRACKET    = 93;
        const long KEY_CARET       = 94;
        const long KEY_UNDERSCORE      = 95;
        const long KEY_BACKQUOTE       = 96;
        const long KEY_a           = 97;
        const long KEY_b           = 98;
        const long KEY_c           = 99;
        const long KEY_d           = 100;
        const long KEY_e           = 101;
        const long KEY_f           = 102;
        const long KEY_g           = 103;
        const long KEY_h           = 104;
        const long KEY_i           = 105;
        const long KEY_j           = 106;
        const long KEY_k           = 107;
        const long KEY_l           = 108;
        const long KEY_m           = 109;
        const long KEY_n           = 110;
        const long KEY_o           = 111;
        const long KEY_p           = 112;
        const long KEY_q           = 113;
        const long KEY_r           = 114;
        const long KEY_s           = 115;
        const long KEY_t           = 116;
        const long KEY_u           = 117;
        const long KEY_v           = 118;
        const long KEY_w           = 119;
        const long KEY_x           = 120;
        const long KEY_y           = 121;
        const long KEY_z           = 122;
        const long KEY_DELETE      = 127;
        // End of ASCII mapped keysyms

        // International keyboard syms 
        const long KEY_WORLD_0     = 160;      /* 0xA0 */
        const long KEY_WORLD_1     = 161;
        const long KEY_WORLD_2     = 162;
        const long KEY_WORLD_3     = 163;
        const long KEY_WORLD_4     = 164;
        const long KEY_WORLD_5     = 165;
        const long KEY_WORLD_6     = 166;
        const long KEY_WORLD_7     = 167;
        const long KEY_WORLD_8     = 168;
        const long KEY_WORLD_9     = 169;
        const long KEY_WORLD_10        = 170;
        const long KEY_WORLD_11        = 171;
        const long KEY_WORLD_12        = 172;
        const long KEY_WORLD_13        = 173;
        const long KEY_WORLD_14        = 174;
        const long KEY_WORLD_15        = 175;
        const long KEY_WORLD_16        = 176;
        const long KEY_WORLD_17        = 177;
        const long KEY_WORLD_18        = 178;
        const long KEY_WORLD_19        = 179;
        const long KEY_WORLD_20        = 180;
        const long KEY_WORLD_21        = 181;
        const long KEY_WORLD_22        = 182;
        const long KEY_WORLD_23        = 183;
        const long KEY_WORLD_24        = 184;
        const long KEY_WORLD_25        = 185;
        const long KEY_WORLD_26        = 186;
        const long KEY_WORLD_27        = 187;
        const long KEY_WORLD_28        = 188;
        const long KEY_WORLD_29        = 189;
        const long KEY_WORLD_30        = 190;
        const long KEY_WORLD_31        = 191;
        const long KEY_WORLD_32        = 192;
        const long KEY_WORLD_33        = 193;
        const long KEY_WORLD_34        = 194;
        const long KEY_WORLD_35        = 195;
        const long KEY_WORLD_36        = 196;
        const long KEY_WORLD_37        = 197;
        const long KEY_WORLD_38        = 198;
        const long KEY_WORLD_39        = 199;
        const long KEY_WORLD_40        = 200;
        const long KEY_WORLD_41        = 201;
        const long KEY_WORLD_42        = 202;
        const long KEY_WORLD_43        = 203;
        const long KEY_WORLD_44        = 204;
        const long KEY_WORLD_45        = 205;
        const long KEY_WORLD_46        = 206;
        const long KEY_WORLD_47        = 207;
        const long KEY_WORLD_48        = 208;
        const long KEY_WORLD_49        = 209;
        const long KEY_WORLD_50        = 210;
        const long KEY_WORLD_51        = 211;
        const long KEY_WORLD_52        = 212;
        const long KEY_WORLD_53        = 213;
        const long KEY_WORLD_54        = 214;
        const long KEY_WORLD_55        = 215;
        const long KEY_WORLD_56        = 216;
        const long KEY_WORLD_57        = 217;
        const long KEY_WORLD_58        = 218;
        const long KEY_WORLD_59        = 219;
        const long KEY_WORLD_60        = 220;
        const long KEY_WORLD_61        = 221;
        const long KEY_WORLD_62        = 222;
        const long KEY_WORLD_63        = 223;
        const long KEY_WORLD_64        = 224;
        const long KEY_WORLD_65        = 225;
        const long KEY_WORLD_66        = 226;
        const long KEY_WORLD_67        = 227;
        const long KEY_WORLD_68        = 228;
        const long KEY_WORLD_69        = 229;
        const long KEY_WORLD_70        = 230;
        const long KEY_WORLD_71        = 231;
        const long KEY_WORLD_72        = 232;
        const long KEY_WORLD_73        = 233;
        const long KEY_WORLD_74        = 234;
        const long KEY_WORLD_75        = 235;
        const long KEY_WORLD_76        = 236;
        const long KEY_WORLD_77        = 237;
        const long KEY_WORLD_78        = 238;
        const long KEY_WORLD_79        = 239;
        const long KEY_WORLD_80        = 240;
        const long KEY_WORLD_81        = 241;
        const long KEY_WORLD_82        = 242;
        const long KEY_WORLD_83        = 243;
        const long KEY_WORLD_84        = 244;
        const long KEY_WORLD_85        = 245;
        const long KEY_WORLD_86        = 246;
        const long KEY_WORLD_87        = 247;
        const long KEY_WORLD_88        = 248;
        const long KEY_WORLD_89        = 249;
        const long KEY_WORLD_90        = 250;
        const long KEY_WORLD_91        = 251;
        const long KEY_WORLD_92        = 252;
        const long KEY_WORLD_93        = 253;
        const long KEY_WORLD_94        = 254;
        const long KEY_WORLD_95        = 255;

        // Numeric keypad
        const long KEY_KP0     = 256;
        const long KEY_KP1     = 257;
        const long KEY_KP2     = 258;
        const long KEY_KP3     = 259;
        const long KEY_KP4     = 260;
        const long KEY_KP5     = 261;
        const long KEY_KP6     = 262;
        const long KEY_KP7     = 263;
        const long KEY_KP8     = 264;
        const long KEY_KP9     = 265;
        const long KEY_KP_PERIOD       = 266;
        const long KEY_KP_DIVIDE       = 267;
        const long KEY_KP_MULTIPLY = 268;
        const long KEY_KP_MINUS        = 269;
        const long KEY_KP_PLUS     = 270;
        const long KEY_KP_ENTER        = 271;
        const long KEY_KP_EQUALS       = 272;

        // Arrows + Home/End pad
        const long KEY_UP          = 273;
        const long KEY_DOWN        = 274;
        const long KEY_RIGHT       = 275;
        const long KEY_LEFT        = 276;
        const long KEY_INSERT      = 277;
        const long KEY_HOME        = 278;
        const long KEY_END     = 279;
        const long KEY_PAGEUP      = 280;
        const long KEY_PAGEDOWN        = 281;

        // Function keys
        const long KEY_F1          = 282;
        const long KEY_F2          = 283;
        const long KEY_F3          = 284;
        const long KEY_F4          = 285;
        const long KEY_F5          = 286;
        const long KEY_F6          = 287;
        const long KEY_F7          = 288;
        const long KEY_F8          = 289;
        const long KEY_F9          = 290;
        const long KEY_F10     = 291;
        const long KEY_F11     = 292;
        const long KEY_F12     = 293;
        const long KEY_F13     = 294;
        const long KEY_F14     = 295;
        const long KEY_F15     = 296;

        // Key state modifier keys
        const long KEY_NUMLOCK     = 300;
        const long KEY_CAPSLOCK        = 301;
        const long KEY_SCROLLOCK       = 302;
        const long KEY_RSHIFT      = 303;
        const long KEY_LSHIFT      = 304;
        const long KEY_RCTRL       = 305;
        const long KEY_LCTRL       = 306;
        const long KEY_RALT        = 307;
        const long KEY_LALT        = 308;
        const long KEY_RMETA       = 309;
        const long KEY_LMETA       = 310;
        const long KEY_LSUPER      = 311;      /* Left "Windows" key */
        const long KEY_RSUPER      = 312;      /* Right "Windows" key */
        const long KEY_MODE        = 313;      /* "Alt Gr" key */
        const long KEY_COMPOSE     = 314;      /* Multi-key compose key */

        // Miscellaneous function keys
        const long KEY_HELP        = 315;
        const long KEY_PRINT       = 316;
        const long KEY_SYSREQ      = 317;
        const long KEY_BREAK       = 318;
        const long KEY_MENU        = 319;
        const long KEY_POWER       = 320;      /* Power Macintosh power key */
        const long KEY_EURO        = 321;      /* Some european keyboards */
        const long KEY_UNDO        = 322;      /* Atari keyboard has Undo */
        // Add any other keys here
    }
}

#endif

