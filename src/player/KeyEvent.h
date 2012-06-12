//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "../api.h"
#include "Event.h"

#include <string>

namespace avg {

class AVG_API KeyEvent : public Event {
    public:
        KeyEvent(Type eventType, unsigned char scanCode, int keyCode, 
                const std::string& keyString, int unicode, int modifiers);
        virtual ~KeyEvent();

        unsigned char getScanCode() const;
        int getKeyCode() const;
        const std::string& getKeyString() const;
        int getUnicode() const;
        int getModifiers() const;

        void trace();

    private: 
        int m_ScanCode; 
        int m_KeyCode;
        std::string m_KeyString;
        int m_Unicode;
        int m_Modifiers;
};

typedef boost::shared_ptr<class KeyEvent> KeyEventPtr;

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
        const long KEY_BackSpace   = 8;
        const long KEY_Tab         = 9;
        const long KEY_Clear       = 12;
        const long KEY_Return      = 13;
        const long KEY_Pause       = 19;
        const long KEY_Escape      = 27;
        const long KEY_space       = 32;
        const long KEY_exclam      = 33;
        const long KEY_quotedbl    = 34;
        const long KEY_numbersign  = 35;
        const long KEY_dollar      = 36;
        const long KEY_ampersand   = 38;
        const long KEY_apostrophe  = 39;
        const long KEY_parenleft   = 40;
        const long KEY_parenright  = 41;
        const long KEY_asterisk    = 42;
        const long KEY_plus        = 43;
        const long KEY_comma       = 44;
        const long KEY_minus       = 45;
        const long KEY_period      = 46;
        const long KEY_slash       = 47;
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
        const long KEY_colon       = 58;
        const long KEY_semicolon   = 59;
        const long KEY_less        = 60;
        const long KEY_equal       = 61;
        const long KEY_greater     = 62;
        const long KEY_question    = 63;
        const long KEY_at          = 64;
        // Skip uppercase letters

        const long KEY_bracketleft     = 91;
        const long KEY_backslash       = 92;
        const long KEY_bracketright    = 93;
        const long KEY_caret           = 94;
        const long KEY_underscore      = 95;
        const long KEY_dead_grave      = 96;
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
        const long KEY_Delete      = 127;
        // End of ASCII mapped keysyms

        // Numeric keypad
        const long KEY_KP_0     = 256;
        const long KEY_KP_1     = 257;
        const long KEY_KP_2     = 258;
        const long KEY_KP_3     = 259;
        const long KEY_KP_4     = 260;
        const long KEY_KP_5     = 261;
        const long KEY_KP_6     = 262;
        const long KEY_KP_7     = 263;
        const long KEY_KP_8     = 264;
        const long KEY_KP_9     = 265;
        const long KEY_KP_Separator    = 266;
        const long KEY_KP_Divide       = 267;
        const long KEY_KP_Multiply     = 268;
        const long KEY_KP_Subtract     = 269;
        const long KEY_KP_Add          = 270;
        const long KEY_KP_Enter        = 271;
        const long KEY_KP_Equal        = 272;

        // Arrows + Home/End pad
        const long KEY_Up          = 273;
        const long KEY_Down        = 274;
        const long KEY_Right       = 275;
        const long KEY_Left        = 276;
        const long KEY_Insert      = 277;
        const long KEY_Home        = 278;
        const long KEY_End         = 279;
        const long KEY_Page_Up     = 280;
        const long KEY_Page_Down   = 281;

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
        const long KEY_F10         = 291;
        const long KEY_F11         = 292;
        const long KEY_F12         = 293;
        const long KEY_F13         = 294;
        const long KEY_F14         = 295;
        const long KEY_F15         = 296;

        // Key state modifier keys
        const long KEY_Num_Lock    = 300;
        const long KEY_Caps_Lock   = 301;
        const long KEY_Scroll_Lock = 302;
        const long KEY_Shift_R     = 303;
        const long KEY_Shift_L     = 304;
        const long KEY_Control_R   = 305;
        const long KEY_Control_L   = 306;
        const long KEY_Alt_R       = 307;
        const long KEY_Alt_L       = 308;
        const long KEY_Meta_L      = 309;
        const long KEY_Meta_R      = 310;
        const long KEY_Super_L     = 311;      /* Left "Windows" key */
        const long KEY_Super_R     = 312;      /* Right "Windows" key */
        const long KEY_ISO_Level3_Shift    = 313;      /* "Alt Gr" key */

        // Miscellaneous function keys
        const long KEY_Help        = 315;
        const long KEY_Print       = 316;
        const long KEY_Sys_Req     = 317;
        const long KEY_Break       = 318;
        const long KEY_Menu        = 319;
        const long KEY_EuroSign    = 321;      /* Some european keyboards */
        const long KEY_Undo        = 322;      /* Atari keyboard has Undo */
        // Add any other keys here
        const long KEY_ssharp      = 323;
        const long KEY_adiaeresis  = 324;
        const long KEY_odiaeresis  = 325;
        const long KEY_udiaeresis  = 326;

    }

}

#endif

