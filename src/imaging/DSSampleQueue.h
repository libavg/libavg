//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _DSSampleQueue_H_
#define _DSSampleQueue_H_

#include "../base/Queue.h"
#include "../base/Point.h"
#include "../graphics/Bitmap.h"

#define _WIN32_DCOM
#include <windows.h>
#include "qedit.h"

namespace avg {

class DSSampleQueue: public ISampleGrabberCB {
public:
    DSSampleQueue(IntPoint Size, PixelFormat cameraPF, PixelFormat destPF, bool bUpsideDown);
    virtual ~DSSampleQueue();

    // ISampleGrabberCB callbacks
    STDMETHODIMP SampleCB(double sampleTime, IMediaSample *pSample);
    
    // Unused
    STDMETHODIMP BufferCB(double sampleTime, BYTE *pBuffer, long BufferLen);

    BitmapPtr getImage(bool bBlock = true);

    // Fake.
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
    
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    
private:
    IntPoint m_Size;
    PixelFormat m_CameraPF;
    PixelFormat m_DestPF;
    bool m_bUpsideDown;
    Queue<Bitmap> m_BitmapQ;
};

}

#endif
