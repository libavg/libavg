//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile$
//   $Author$
//   $Revision$
//   $Date$
//
//
//=============================================================================

#include "SDLEventSource.h"
#include "MouseEvent.h"
#include "WindowEvent.h"
#include "AxisEvent.h"
#include "KeyEvent.h"
#include "ButtonEvent.h"
#include "KeyCodes.h"

#include <iostream>

using namespace std;

#define DB(x) // x;

namespace input {

    // SDLKey sym-code to AC KeyCode translation table
    std::vector<KeyCode> SDLEventSource::myKeyCodeTranslationTable(SDLK_LAST, KEY_UNKNOWN);

    SDLEventSource::SDLEventSource() {
        initTranslationTable();
    }

    void SDLEventSource::init() {
#ifndef WIN32
        initJoysticks();
#endif
    }

    vector<EventPtr>
    SDLEventSource::poll() {
        SDL_Event sdlEvent;
        vector<EventPtr> myEvents;

        while(SDL_PollEvent(&sdlEvent)){
            switch(sdlEvent.type){
                case SDL_MOUSEMOTION:
                    myEvents.push_back(createMouseMotionEvent(Event::MOUSE_MOTION, sdlEvent));
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    myEvents.push_back(createMouseButtonEvent(Event::MOUSE_BUTTON_DOWN, sdlEvent));
                    break;
                case SDL_MOUSEBUTTONUP:
                    myEvents.push_back(createMouseButtonEvent(Event::MOUSE_BUTTON_UP, sdlEvent));
                    break;
                case SDL_JOYAXISMOTION:
                    myEvents.push_back(createAxisEvent(sdlEvent));
                    break;
                case SDL_JOYBUTTONDOWN:
                    myEvents.push_back(createButtonEvent(Event::BUTTON_DOWN, sdlEvent));
                    break;
                case SDL_JOYBUTTONUP:
                    myEvents.push_back(createButtonEvent(Event::BUTTON_UP, sdlEvent));
                    break;
                case SDL_KEYDOWN:
                    myEvents.push_back(createKeyEvent(Event::KEY_DOWN, sdlEvent));
                    break;
                case SDL_KEYUP:
                    myEvents.push_back(createKeyEvent(Event::KEY_UP, sdlEvent));
                    break;
                case SDL_QUIT:
                    myEvents.push_back(EventPtr(new Event(Event::QUIT)));
                    break;
                case SDL_VIDEORESIZE:
                    myEvents.push_back(EventPtr(
                            new WindowEvent(Event::RESIZE,sdlEvent.resize.w,sdlEvent.resize.h)));
                    break;
                default:
                    DB(cerr << "Skipping Unknown event type" << endl;);
            }
        }
        return myEvents;
    }

    EventPtr
    SDLEventSource::createMouseMotionEvent(Event::Type theType, const SDL_Event & theSDLEvent) {
        return EventPtr(new MouseEvent(theType, theSDLEvent.motion.state & SDL_BUTTON(1),
            theSDLEvent.motion.state & SDL_BUTTON(3), theSDLEvent.motion.state & SDL_BUTTON(2),
            theSDLEvent.motion.x, theSDLEvent.motion.y, theSDLEvent.motion.xrel, theSDLEvent.motion.yrel));
    }

    EventPtr
    SDLEventSource::createMouseButtonEvent(Event::Type theType, const SDL_Event & theSDLEvent) {
        MouseEvent::Button myButton;
        switch (theSDLEvent.button.button) {
            case SDL_BUTTON_LEFT:
                myButton = MouseEvent::Left;
                break;
            case SDL_BUTTON_MIDDLE:
                myButton = MouseEvent::Middle;
                break;
            case SDL_BUTTON_RIGHT:
                myButton = MouseEvent::Right;
                break;
        }
        return EventPtr(new MouseEvent(theType, theSDLEvent.button.button == SDL_BUTTON_LEFT,
            theSDLEvent.button.button == SDL_BUTTON_MIDDLE, theSDLEvent.button.button == SDL_BUTTON_RIGHT,
            theSDLEvent.button.x, theSDLEvent.button.y, 0, 0, myButton));
    }

    EventPtr
    SDLEventSource::createAxisEvent(const SDL_Event & theSDLEvent) {
        return EventPtr(new AxisEvent(theSDLEvent.jaxis.which, theSDLEvent.jaxis.axis,
                theSDLEvent.jaxis.value));
    }
    
    EventPtr
    SDLEventSource::createButtonEvent(Event::Type theType, const SDL_Event & theSDLEvent) {
        return EventPtr(new ButtonEvent(theType, theSDLEvent.jbutton.which,
                theSDLEvent.jbutton.button));
    }

    EventPtr
    SDLEventSource::createKeyEvent(Event::Type theType, const SDL_Event & theSDLEvent) {
        KeyCode myKeyCode = myKeyCodeTranslationTable[theSDLEvent.key.keysym.sym];
        unsigned int myModifiers = KEYMOD_NONE;

        if (theSDLEvent.key.keysym.mod & KMOD_LSHIFT) { myModifiers |= KEYMOD_LSHIFT; }
        if (theSDLEvent.key.keysym.mod & KMOD_RSHIFT) { myModifiers |= KEYMOD_RSHIFT; }
        if (theSDLEvent.key.keysym.mod & KMOD_LCTRL) { myModifiers |= KEYMOD_LCTRL; }
        if (theSDLEvent.key.keysym.mod & KMOD_RCTRL) { myModifiers |= KEYMOD_RCTRL; }
        if (theSDLEvent.key.keysym.mod & KMOD_LALT) { myModifiers |= KEYMOD_LALT; }
        if (theSDLEvent.key.keysym.mod & KMOD_RALT) { myModifiers |= KEYMOD_RALT; }
        if (theSDLEvent.key.keysym.mod & KMOD_LMETA) { myModifiers |= KEYMOD_LMETA; }
        if (theSDLEvent.key.keysym.mod & KMOD_RMETA) { myModifiers |= KEYMOD_RMETA; }
        if (theSDLEvent.key.keysym.mod & KMOD_NUM) { myModifiers |= KEYMOD_NUM; }
        if (theSDLEvent.key.keysym.mod & KMOD_CAPS) { myModifiers |= KEYMOD_CAPS; }
        if (theSDLEvent.key.keysym.mod & KMOD_MODE) { myModifiers |= KEYMOD_MODE; }
        if (theSDLEvent.key.keysym.mod & KMOD_RESERVED) { myModifiers |= KEYMOD_RESERVED; }

        return EventPtr(new KeyEvent(theType, theSDLEvent.key.keysym.scancode, myKeyCode,
                SDL_GetKeyName(theSDLEvent.key.keysym.sym), myModifiers));
    }

    void
    SDLEventSource::initJoysticks() {
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
        SDL_JoystickEventState(SDL_ENABLE);
        cerr << "**** Number of joysticks: " << SDL_NumJoysticks() << endl;
        for (int i=0; i<SDL_NumJoysticks(); i++) {
            SDL_Joystick * myJoystick = SDL_JoystickOpen(i);
            if (!myJoystick) {
                cerr << "Warning: could not open joystick # " << i << "of"
                    << SDL_NumJoysticks() << endl;
            } else {
                printf("Opened Joystick %i\n", i);
                printf("Name: %s\n", SDL_JoystickName(i));
                printf("Number of Axes: %d\n", SDL_JoystickNumAxes(myJoystick));
                printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(myJoystick));
                printf("Number of Balls: %d\n", SDL_JoystickNumBalls(myJoystick));
            }

        }
    }


    void
    SDLEventSource::initTranslationTable() {
        #define TRANSLATION_ENTRY(x) myKeyCodeTranslationTable[SDLK_##x] = KEY_##x;

        TRANSLATION_ENTRY(UNKNOWN);
        TRANSLATION_ENTRY(BACKSPACE);
        TRANSLATION_ENTRY(TAB);
        TRANSLATION_ENTRY(CLEAR);
        TRANSLATION_ENTRY(RETURN);
        TRANSLATION_ENTRY(PAUSE);
        TRANSLATION_ENTRY(ESCAPE);
        TRANSLATION_ENTRY(SPACE);
        TRANSLATION_ENTRY(EXCLAIM);
        TRANSLATION_ENTRY(QUOTEDBL);
        TRANSLATION_ENTRY(HASH);
        TRANSLATION_ENTRY(DOLLAR);
        TRANSLATION_ENTRY(AMPERSAND);
        TRANSLATION_ENTRY(QUOTE);
        TRANSLATION_ENTRY(LEFTPAREN);
        TRANSLATION_ENTRY(RIGHTPAREN);
        TRANSLATION_ENTRY(ASTERISK);
        TRANSLATION_ENTRY(PLUS);
        TRANSLATION_ENTRY(COMMA);
        TRANSLATION_ENTRY(MINUS);
        TRANSLATION_ENTRY(PERIOD);
        TRANSLATION_ENTRY(SLASH);
        TRANSLATION_ENTRY(0);
        TRANSLATION_ENTRY(1);
        TRANSLATION_ENTRY(2);
        TRANSLATION_ENTRY(3);
        TRANSLATION_ENTRY(4);
        TRANSLATION_ENTRY(5);
        TRANSLATION_ENTRY(6);
        TRANSLATION_ENTRY(7);
        TRANSLATION_ENTRY(8);
        TRANSLATION_ENTRY(9);
        TRANSLATION_ENTRY(COLON);
        TRANSLATION_ENTRY(SEMICOLON);
        TRANSLATION_ENTRY(LESS);
        TRANSLATION_ENTRY(EQUALS);
        TRANSLATION_ENTRY(GREATER);
        TRANSLATION_ENTRY(QUESTION);
        TRANSLATION_ENTRY(AT);
        TRANSLATION_ENTRY(LEFTBRACKET);
        TRANSLATION_ENTRY(BACKSLASH);
        TRANSLATION_ENTRY(RIGHTBRACKET);
        TRANSLATION_ENTRY(CARET);
        TRANSLATION_ENTRY(UNDERSCORE);
        TRANSLATION_ENTRY(BACKQUOTE);
        TRANSLATION_ENTRY(a);
        TRANSLATION_ENTRY(b);
        TRANSLATION_ENTRY(c);
        TRANSLATION_ENTRY(d);
        TRANSLATION_ENTRY(e);
        TRANSLATION_ENTRY(f);
        TRANSLATION_ENTRY(g);
        TRANSLATION_ENTRY(h);
        TRANSLATION_ENTRY(i);
        TRANSLATION_ENTRY(j);
        TRANSLATION_ENTRY(k);
        TRANSLATION_ENTRY(l);
        TRANSLATION_ENTRY(m);
        TRANSLATION_ENTRY(n);
        TRANSLATION_ENTRY(o);
        TRANSLATION_ENTRY(p);
        TRANSLATION_ENTRY(q);
        TRANSLATION_ENTRY(r);
        TRANSLATION_ENTRY(s);
        TRANSLATION_ENTRY(t);
        TRANSLATION_ENTRY(u);
        TRANSLATION_ENTRY(v);
        TRANSLATION_ENTRY(w);
        TRANSLATION_ENTRY(x);
        TRANSLATION_ENTRY(y);
        TRANSLATION_ENTRY(z);
        TRANSLATION_ENTRY(DELETE);
        TRANSLATION_ENTRY(WORLD_0);
        TRANSLATION_ENTRY(WORLD_1);
        TRANSLATION_ENTRY(WORLD_2);
        TRANSLATION_ENTRY(WORLD_3);
        TRANSLATION_ENTRY(WORLD_4);
        TRANSLATION_ENTRY(WORLD_5);
        TRANSLATION_ENTRY(WORLD_6);
        TRANSLATION_ENTRY(WORLD_7);
        TRANSLATION_ENTRY(WORLD_8);
        TRANSLATION_ENTRY(WORLD_9);
        TRANSLATION_ENTRY(WORLD_10);
        TRANSLATION_ENTRY(WORLD_11);
        TRANSLATION_ENTRY(WORLD_12);
        TRANSLATION_ENTRY(WORLD_13);
        TRANSLATION_ENTRY(WORLD_14);
        TRANSLATION_ENTRY(WORLD_15);
        TRANSLATION_ENTRY(WORLD_16);
        TRANSLATION_ENTRY(WORLD_17);
        TRANSLATION_ENTRY(WORLD_18);
        TRANSLATION_ENTRY(WORLD_19);
        TRANSLATION_ENTRY(WORLD_20);
        TRANSLATION_ENTRY(WORLD_21);
        TRANSLATION_ENTRY(WORLD_22);
        TRANSLATION_ENTRY(WORLD_23);
        TRANSLATION_ENTRY(WORLD_24);
        TRANSLATION_ENTRY(WORLD_25);
        TRANSLATION_ENTRY(WORLD_26);
        TRANSLATION_ENTRY(WORLD_27);
        TRANSLATION_ENTRY(WORLD_28);
        TRANSLATION_ENTRY(WORLD_29);
        TRANSLATION_ENTRY(WORLD_30);
        TRANSLATION_ENTRY(WORLD_31);
        TRANSLATION_ENTRY(WORLD_32);
        TRANSLATION_ENTRY(WORLD_33);
        TRANSLATION_ENTRY(WORLD_34);
        TRANSLATION_ENTRY(WORLD_35);
        TRANSLATION_ENTRY(WORLD_36);
        TRANSLATION_ENTRY(WORLD_37);
        TRANSLATION_ENTRY(WORLD_38);
        TRANSLATION_ENTRY(WORLD_39);
        TRANSLATION_ENTRY(WORLD_40);
        TRANSLATION_ENTRY(WORLD_41);
        TRANSLATION_ENTRY(WORLD_42);
        TRANSLATION_ENTRY(WORLD_43);
        TRANSLATION_ENTRY(WORLD_44);
        TRANSLATION_ENTRY(WORLD_45);
        TRANSLATION_ENTRY(WORLD_46);
        TRANSLATION_ENTRY(WORLD_47);
        TRANSLATION_ENTRY(WORLD_48);
        TRANSLATION_ENTRY(WORLD_49);
        TRANSLATION_ENTRY(WORLD_50);
        TRANSLATION_ENTRY(WORLD_51);
        TRANSLATION_ENTRY(WORLD_52);
        TRANSLATION_ENTRY(WORLD_53);
        TRANSLATION_ENTRY(WORLD_54);
        TRANSLATION_ENTRY(WORLD_55);
        TRANSLATION_ENTRY(WORLD_56);
        TRANSLATION_ENTRY(WORLD_57);
        TRANSLATION_ENTRY(WORLD_58);
        TRANSLATION_ENTRY(WORLD_59);
        TRANSLATION_ENTRY(WORLD_60);
        TRANSLATION_ENTRY(WORLD_61);
        TRANSLATION_ENTRY(WORLD_62);
        TRANSLATION_ENTRY(WORLD_63);
        TRANSLATION_ENTRY(WORLD_64);
        TRANSLATION_ENTRY(WORLD_65);
        TRANSLATION_ENTRY(WORLD_66);
        TRANSLATION_ENTRY(WORLD_67);
        TRANSLATION_ENTRY(WORLD_68);
        TRANSLATION_ENTRY(WORLD_69);
        TRANSLATION_ENTRY(WORLD_70);
        TRANSLATION_ENTRY(WORLD_71);
        TRANSLATION_ENTRY(WORLD_72);
        TRANSLATION_ENTRY(WORLD_73);
        TRANSLATION_ENTRY(WORLD_74);
        TRANSLATION_ENTRY(WORLD_75);
        TRANSLATION_ENTRY(WORLD_76);
        TRANSLATION_ENTRY(WORLD_77);
        TRANSLATION_ENTRY(WORLD_78);
        TRANSLATION_ENTRY(WORLD_79);
        TRANSLATION_ENTRY(WORLD_80);
        TRANSLATION_ENTRY(WORLD_81);
        TRANSLATION_ENTRY(WORLD_82);
        TRANSLATION_ENTRY(WORLD_83);
        TRANSLATION_ENTRY(WORLD_84);
        TRANSLATION_ENTRY(WORLD_85);
        TRANSLATION_ENTRY(WORLD_86);
        TRANSLATION_ENTRY(WORLD_87);
        TRANSLATION_ENTRY(WORLD_88);
        TRANSLATION_ENTRY(WORLD_89);
        TRANSLATION_ENTRY(WORLD_90);
        TRANSLATION_ENTRY(WORLD_91);
        TRANSLATION_ENTRY(WORLD_92);
        TRANSLATION_ENTRY(WORLD_93);
        TRANSLATION_ENTRY(WORLD_94);
        TRANSLATION_ENTRY(WORLD_95);
        TRANSLATION_ENTRY(KP0);
        TRANSLATION_ENTRY(KP1);
        TRANSLATION_ENTRY(KP2);
        TRANSLATION_ENTRY(KP3);
        TRANSLATION_ENTRY(KP4);
        TRANSLATION_ENTRY(KP5);
        TRANSLATION_ENTRY(KP6);
        TRANSLATION_ENTRY(KP7);
        TRANSLATION_ENTRY(KP8);
        TRANSLATION_ENTRY(KP9);
        TRANSLATION_ENTRY(KP_PERIOD);
        TRANSLATION_ENTRY(KP_DIVIDE);
        TRANSLATION_ENTRY(KP_MULTIPLY);
        TRANSLATION_ENTRY(KP_MINUS);
        TRANSLATION_ENTRY(KP_PLUS);
        TRANSLATION_ENTRY(KP_ENTER);
        TRANSLATION_ENTRY(KP_EQUALS);
        TRANSLATION_ENTRY(UP);
        TRANSLATION_ENTRY(DOWN);
        TRANSLATION_ENTRY(RIGHT);
        TRANSLATION_ENTRY(LEFT);
        TRANSLATION_ENTRY(INSERT);
        TRANSLATION_ENTRY(HOME);
        TRANSLATION_ENTRY(END);
        TRANSLATION_ENTRY(PAGEUP);
        TRANSLATION_ENTRY(PAGEDOWN);
        TRANSLATION_ENTRY(F1);
        TRANSLATION_ENTRY(F2);
        TRANSLATION_ENTRY(F3);
        TRANSLATION_ENTRY(F4);
        TRANSLATION_ENTRY(F5);
        TRANSLATION_ENTRY(F6);
        TRANSLATION_ENTRY(F7);
        TRANSLATION_ENTRY(F8);
        TRANSLATION_ENTRY(F9);
        TRANSLATION_ENTRY(F10);
        TRANSLATION_ENTRY(F11);
        TRANSLATION_ENTRY(F12);
        TRANSLATION_ENTRY(F13);
        TRANSLATION_ENTRY(F14);
        TRANSLATION_ENTRY(F15);
        TRANSLATION_ENTRY(NUMLOCK);
        TRANSLATION_ENTRY(CAPSLOCK);
        TRANSLATION_ENTRY(SCROLLOCK);
        TRANSLATION_ENTRY(RSHIFT);
        TRANSLATION_ENTRY(LSHIFT);
        TRANSLATION_ENTRY(RCTRL);
        TRANSLATION_ENTRY(LCTRL);
        TRANSLATION_ENTRY(RALT);
        TRANSLATION_ENTRY(LALT);
        TRANSLATION_ENTRY(RMETA);
        TRANSLATION_ENTRY(LMETA);
        TRANSLATION_ENTRY(LSUPER);
        TRANSLATION_ENTRY(RSUPER);
        TRANSLATION_ENTRY(MODE);
        TRANSLATION_ENTRY(COMPOSE);
        TRANSLATION_ENTRY(HELP);
        TRANSLATION_ENTRY(PRINT);
        TRANSLATION_ENTRY(SYSREQ);
        TRANSLATION_ENTRY(BREAK);
        TRANSLATION_ENTRY(MENU);
        TRANSLATION_ENTRY(POWER);
        TRANSLATION_ENTRY(EURO);
        TRANSLATION_ENTRY(UNDO);
    }

}

