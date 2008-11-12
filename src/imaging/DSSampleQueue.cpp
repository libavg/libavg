// Copyright (C) 2008 Archimedes Solutions GmbH,
// Saarbrücker Str. 24b, Berlin, Germany
//
// This file contains proprietary source code and confidential
// information. Its contents may not be disclosed or distributed to
// third parties unless prior specific permission by Archimedes
// Solutions GmbH, Berlin, Germany is obtained in writing. This applies
// to copies made in any form and using any medium. It applies to
// partial as well as complete copies.

#include "DSSampleQueue.h"
#include "DSHelper.h"

#include <dshow.h>

namespace avg {

DSSampleQueue::DSSampleQueue(IntPoint Size, PixelFormat CameraPF, PixelFormat DestPF)
    : m_Size(Size),
      m_CameraPF(CameraPF),
      m_DestPF(DestPF)
{
}

DSSampleQueue::~DSSampleQueue()
{
}

STDMETHODIMP DSSampleQueue::SampleCB(double SampleTime, IMediaSample *pSample)
{
    unsigned char * pData;

    // Get the current image.
    pSample->GetPointer(&pData);
    int Stride = m_Size.x*Bitmap::getBytesPerPixel(m_CameraPF);
    Bitmap CamBmp(m_Size, m_CameraPF, pData, Stride, false, "CameraImage");

    // Copy over to bitmap queue, doing pixel format conversion if necessary.
    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(m_Size, m_DestPF, 
            "ConvertedCameraImage"));
    pDestBmp->copyPixels(CamBmp);
    m_BitmapQ.push(pDestBmp);

    return S_OK;
}

STDMETHODIMP DSSampleQueue::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
    return E_NOTIMPL;
}

BitmapPtr DSSampleQueue::getImage(bool bBlock)
{
    // Image is upside-down and contains an invalid alpha channel at this point.
    return m_BitmapQ.pop(bBlock);
}

STDMETHODIMP DSSampleQueue::QueryInterface(REFIID riid, void ** ppv)
{
    if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) {
        *ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
        return NOERROR;
    }    
    return E_NOINTERFACE;
}

}
