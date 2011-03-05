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

#include "DSSampleQueue.h"
#include "DSHelper.h"

#include <dshow.h>

namespace avg {

DSSampleQueue::DSSampleQueue(IntPoint size, PixelFormat cameraPF, PixelFormat destPF)
    : m_Size(size),
      m_CameraPF(cameraPF),
      m_DestPF(destPF)
{
}

DSSampleQueue::~DSSampleQueue()
{
}

STDMETHODIMP DSSampleQueue::SampleCB(double sampleTime, IMediaSample *pSample)
{
    unsigned char * pData;

    // Get the current image.
    pSample->GetPointer(&pData);
    int stride = m_Size.x*Bitmap::getBytesPerPixel(m_CameraPF);
    Bitmap camBmp(m_Size, m_CameraPF, pData, stride, false, "CameraImage");

    // Copy over to bitmap queue, doing pixel format conversion if necessary.
    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(m_Size, m_DestPF, 
            "ConvertedCameraImage"));
    pDestBmp->copyPixels(camBmp);
    m_BitmapQ.push(pDestBmp);

    return S_OK;
}

STDMETHODIMP DSSampleQueue::BufferCB(double sampleTime, BYTE *pBuffer, long BufferLen)
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
