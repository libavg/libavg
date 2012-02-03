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

#include "DSSampleGrabber.h"

#include <initguid.h>

#pragma warning(disable: 4800)

// {455A53B7-FC34-4960-94CE-A17A0B23F807}
DEFINE_GUID(CLSID_libavgGrabber, 
0x455a53b7, 0xfc34, 0x4960, 0x94, 0xce, 0xa1, 0x7a, 0xb, 0x23, 0xf8, 0x7);

namespace avg {

class CSampleGrabberAllocator : public CMemAllocator
{
public:
    CSampleGrabberAllocator(CSampleGrabberInPin* pParent, HRESULT* phr);
    ~CSampleGrabberAllocator();

    HRESULT Alloc();
    void ReallyFree();
    STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES* pRequest, 
            ALLOCATOR_PROPERTIES* pActual);

protected:
    CSampleGrabberInPin * m_pPin;

private:
    friend class CSampleGrabberInPin;
    friend class CSampleGrabber;
};

class CSampleGrabberInPin : public CTransInPlaceInputPin
{
public:
    CSampleGrabberInPin(CTransInPlaceFilter* pFilter, HRESULT* pHr);
    ~CSampleGrabberInPin();

    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);

    // override this or GetMediaType is never called
    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes** ppEnum);

    // we override this to tell whoever's upstream of us what kind of
    // properties we're going to demand to have
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);

private:
    friend class CSampleGrabberAllocator;
    friend class CSampleGrabber;

    CSampleGrabberAllocator* m_pPrivateAllocator;
    ALLOCATOR_PROPERTIES m_allocprops;
    BYTE* m_pBuffer;
    BOOL m_bMediaTypeChanged;
};

CSampleGrabber::CSampleGrabber(IUnknown * pOuter, HRESULT * phr)
    : CTransInPlaceFilter(TEXT("libavg sample grabber"), (IUnknown*) pOuter, 
              CLSID_libavgGrabber, phr, false),
      m_pCallback(NULL)
{
    m_pInput = (CTransInPlaceInputPin*) new CSampleGrabberInPin(this, phr);
    
    IPin *pOutput = GetPin(1);
}

STDMETHODIMP CSampleGrabber::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    AVG_ASSERT(ppv);

    if (riid == IID_IlibavgGrabber) {                
        return GetInterface((IlibavgGrabber *)this, ppv);
    } else {
        return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

HRESULT CSampleGrabber::CheckInputType(const CMediaType* pmt)
{
    AVG_ASSERT(pmt);
    CAutoLock lock(&m_Lock);

    if (MEDIATYPE_Video == *pmt->Type( )) {
        return NOERROR;
    } else {
        return VFW_E_INVALID_MEDIA_TYPE;
    }
}

HRESULT CSampleGrabber::Receive(IMediaSample * pms)
{
    AVG_ASSERT(pms);

    HRESULT hr;
    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();

    if (pProps->dwStreamId != AM_STREAM_MEDIA) {
        if (m_pOutput->IsConnected()) {
            return m_pOutput->Deliver(pms);
        } else {
            return NOERROR;
        }
    }
    if (UsingDifferentAllocators()) {
        pms = Copy(pms);
        AVG_ASSERT(pms);
    }

    hr = Transform(pms);
    AVG_ASSERT(!FAILED(hr));
    hr = m_pOutput->Deliver(pms);
    if (UsingDifferentAllocators()) {
        pms->Release();
    }
    return hr;
}

HRESULT CSampleGrabber::Transform(IMediaSample* pms)
{
    AVG_ASSERT(pms);
    CAutoLock lock(&m_Lock);

    if (m_pCallback) {
        REFERENCE_TIME StartTime;
        REFERENCE_TIME StopTime;
        pms->GetTime( &StartTime, &StopTime);

        StartTime += m_pInput->CurrentStartTime();
        StopTime  += m_pInput->CurrentStartTime();

        m_pCallback->onSample(pms);
    }
    return NOERROR;
}

void STDMETHODCALLTYPE CSampleGrabber::SetCallback(IDSSampleCallback* pCallback)
{
    CAutoLock lock( &m_Lock );
    m_pCallback = pCallback;
}

CSampleGrabberInPin::CSampleGrabberInPin(CTransInPlaceFilter* pFilter, HRESULT* pHr) 
    : CTransInPlaceInputPin(TEXT("SampleGrabberInputPin\0"), pFilter, pHr, L"Input\0"),
      m_pPrivateAllocator(NULL),
      m_pBuffer(NULL),
      m_bMediaTypeChanged(FALSE)
{
    memset(&m_allocprops, 0, sizeof(m_allocprops));
}

CSampleGrabberInPin::~CSampleGrabberInPin()
{
    if (m_pPrivateAllocator) {
        delete m_pPrivateAllocator;
    }
}

HRESULT CSampleGrabberInPin::GetMediaType(int iPosition, CMediaType* pMediaType)
{
    AVG_ASSERT(pMediaType);
    AVG_ASSERT(iPosition >= 0);
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pMediaType = CMediaType();
    pMediaType->SetType(&MEDIATYPE_Video);

    return S_OK;
}

STDMETHODIMP CSampleGrabberInPin::EnumMediaTypes(IEnumMediaTypes** ppEnum) 
{
    AVG_ASSERT(ppEnum);
    ValidateReadWritePtr(ppEnum, sizeof(IEnumMediaTypes *));

    if(!((CSampleGrabber*)m_pTIPFilter)->OutputPin()->IsConnected())
    {
        *ppEnum = new CEnumMediaTypes(this, NULL);
        return NOERROR;
    }

    return ((CSampleGrabber*)m_pTIPFilter)->OutputPin()->GetConnected()
            ->EnumMediaTypes(ppEnum);
}

HRESULT CSampleGrabberInPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    AVG_ASSERT(pProps);

    if (m_pPrivateAllocator) {
        *pProps = m_allocprops;
        return S_OK;
    } else {
        return CTransInPlaceInputPin::GetAllocatorRequirements(pProps);
    }
}

}