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
#ifndef _Y60_INPUT_SDLEVENTSOURCE_INCLUDED_
#define _Y60_INPUT_SDLEVENTSOURCE_INCLUDED_

#include "Event.h"
#include "IEventSource.h"
#include "KeyCodes.h"

#include <SDL/SDL.h>
#include <asl/Ptr.h>

#include <vector>


namespace input {

    class SDLEventSource : public IEventSource {
        public:
            SDLEventSource();
            virtual void init();
            virtual std::vector<EventPtr> poll();
        private:
            static std::vector<KeyCode> myKeyCodeTranslationTable;
            EventPtr createMouseMotionEvent(Event::Type theType, const SDL_Event & theSDLEvent);
            EventPtr createMouseButtonEvent(Event::Type theType, const SDL_Event & theSDLEvent);
            EventPtr createAxisEvent(const SDL_Event & theSDLEvent);
            EventPtr createButtonEvent(Event::Type theType, const SDL_Event & theSDLEvent);
            EventPtr createKeyEvent(Event::Type theType, const SDL_Event & theSDLEvent);
            void initJoysticks();
            void initTranslationTable();
    };
}

#endif
