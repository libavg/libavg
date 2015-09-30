//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "SDLTouchInputDevice.h"
#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "TouchStatus.h"
#include "TouchEvent.h"
#include "SDLWindow.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"
#include "../base/ConfigMgr.h"

using namespace std;

namespace avg {

SDLTouchInputDevice::SDLTouchInputDevice(const DivNodePtr& pEventReceiverNode)
    : MultitouchInputDevice(pEventReceiverNode)
{
    // TODO: This doesn't look robust.
    m_ClientAreaOffset = IntPoint(GetSystemMetrics(SM_CYBORDER)+2, GetSystemMetrics(SM_CYCAPTION)+3);
}

void SDLTouchInputDevice::onTouchEvent(SDLWindow* pWindow, const SDL_Event& sdlEvent)
{
    EventPtr pNewEvent;
    AVG_ASSERT(sdlEvent.type == SDL_FINGERDOWN || sdlEvent.type == SDL_FINGERMOTION 
            || sdlEvent.type == SDL_FINGERUP);
    SDL_TouchFingerEvent fingerEvent = sdlEvent.tfinger;
    glm::vec2 normPos(fingerEvent.x, fingerEvent.y);
    IntPoint screenPos = normPos * getTouchArea();
    RECT winRect;
    bool bOk = GetWindowRect(pWindow->getWinHWnd(), &winRect);
    AVG_ASSERT(bOk);
    IntPoint pos = screenPos - IntPoint(winRect.left, winRect.top)+m_ClientAreaOffset;
    IntPoint winSize = IntPoint(Player::get()->getRootNode()->getSize());
    pos.x = min(max(pos.x, 0), winSize.x-1);
    pos.y = min(max(pos.y, 0), winSize.y-1);
    switch (sdlEvent.type) {
        case SDL_FINGERDOWN:
            {
                cerr << "down: " << pos << endl;
                TouchEventPtr pEvent (new TouchEvent(getNextContactID(), 
                        Event::CURSOR_DOWN, pos, Event::TOUCH));
                addTouchStatus(fingerEvent.fingerId, pEvent);
            }
            break;
        case SDL_FINGERUP:
            {
                cerr << "up: " << pos << endl;
                TouchStatusPtr pTouchStatus = getTouchStatus(fingerEvent.fingerId);
                CursorEventPtr pOldEvent = pTouchStatus->getLastEvent();
                TouchEventPtr pUpEvent(new TouchEvent(pOldEvent->getCursorID(), 
                        Event::CURSOR_UP, pos, Event::TOUCH));
                pTouchStatus->pushEvent(pUpEvent);
            }
            break;
        case SDL_FINGERMOTION:
            {
                cerr << "motion: " << pos << endl;
                TouchEventPtr pEvent(new TouchEvent(0, Event::CURSOR_MOTION, pos,
                        Event::TOUCH));
                TouchStatusPtr pTouchStatus = getTouchStatus(fingerEvent.fingerId);
                AVG_ASSERT(pTouchStatus);
                pTouchStatus->pushEvent(pEvent);
            }
            break;
		default:
            AVG_ASSERT(false);
            break;
	}
}

}
