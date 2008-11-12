// Copyright (C) 2008 Archimedes Solutions GmbH,
// Saarbrücker Str. 24b, Berlin, Germany
//
// This file contains proprietary source code and confidential
// information. Its contents may not be disclosed or distributed to
// third parties unless prior specific permission by Archimedes
// Solutions GmbH, Berlin, Germany is obtained in writing. This applies
// to copies made in any form and using any medium. It applies to
// partial as well as complete copies.

#ifndef _DSSampleQueue_H_
#define _DSSampleQueue_H_

#include "../base/Queue.h"
#include "../base/Point.h"
#include "../graphics/Bitmap.h"

#define _WIN32_DCOM
#include <winsock2.h>
#include <windows.h>
#include <Qedit.h>

namespace avg {

class DSSampleQueue: public ISampleGrabberCB {
public:
    DSSampleQueue(IntPoint Size, PixelFormat CameraPF, PixelFormat DestPF);
    virtual ~DSSampleQueue();

    // ISampleGrabberCB callbacks
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample);
	
    // Unused
    STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);

	BitmapPtr getImage(bool bBlock = true);

    // Fake.
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
    
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    
private:
    IntPoint m_Size;
    PixelFormat m_CameraPF;
    PixelFormat m_DestPF;
    Queue<BitmapPtr> m_BitmapQ;
};

}

#endif
