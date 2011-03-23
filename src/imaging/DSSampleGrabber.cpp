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

#include "DSSampleGrabber.h"

#include "../base/Exception.h"

#include <initguid.h>

#include "qedit.h"

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

    // override this to refuse any allocators besides
    // the one the user wants, if this is set
    STDMETHODIMP NotifyAllocator(IMemAllocator* pAllocator, BOOL bReadOnly);

    // override this so we always return the special allocator, if necessary
    STDMETHODIMP GetAllocator(IMemAllocator** ppAllocator);

    // we override this to tell whoever's upstream of us what kind of
    // properties we're going to demand to have
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);

protected:
    HRESULT SetDeliveryBuffer(ALLOCATOR_PROPERTIES props, BYTE* m_pBuffer);

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

STDMETHODIMP CSampleGrabber::SetDeliveryBuffer(ALLOCATOR_PROPERTIES props, BYTE* pBuffer)
{
    AVG_ASSERT(InputPin());
    AVG_ASSERT(OutputPin());
    AVG_ASSERT(!InputPin()->IsConnected());
    AVG_ASSERT(!OutputPin()->IsConnected());

    return ((CSampleGrabberInPin*)m_pInput)->SetDeliveryBuffer(props, pBuffer);
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

STDMETHODIMP CSampleGrabberInPin::NotifyAllocator(IMemAllocator* pAllocator, 
        BOOL bReadOnly)
{
    if (m_pPrivateAllocator) {
        if (pAllocator != m_pPrivateAllocator) {
            return E_FAIL;
        }
    }
    return CTransInPlaceInputPin::NotifyAllocator(pAllocator, bReadOnly);
}

STDMETHODIMP CSampleGrabberInPin::GetAllocator(IMemAllocator** ppAllocator)
{
    if (m_pPrivateAllocator) {
        AVG_ASSERT(ppAllocator);

        *ppAllocator = m_pPrivateAllocator;
        m_pPrivateAllocator->AddRef();
        return NOERROR;
    } else {
        return CTransInPlaceInputPin::GetAllocator(ppAllocator);
    }
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

HRESULT CSampleGrabberInPin::SetDeliveryBuffer(ALLOCATOR_PROPERTIES props, BYTE* pBuffer)
{
    AVG_ASSERT(props.cBuffers == 1);
    AVG_ASSERT(pBuffer);

    m_allocprops = props;
    m_pBuffer = pBuffer;

    // If there is an existing allocator, make sure that it is released
    // to prevent a memory leak
    if (m_pPrivateAllocator) {
        m_pPrivateAllocator->Release();
        m_pPrivateAllocator = NULL;
    }

    HRESULT hr = S_OK;
    m_pPrivateAllocator = new CSampleGrabberAllocator(this, &hr);
    m_pPrivateAllocator->AddRef();
    return hr;
}

CSampleGrabberAllocator::CSampleGrabberAllocator(CSampleGrabberInPin* pParent, 
        HRESULT* phr) 
    : CMemAllocator(TEXT("SampleGrabberAllocator\0"), NULL, phr),
      m_pPin(pParent)
{
}

CSampleGrabberAllocator::~CSampleGrabberAllocator()
{
    m_pBuffer = NULL;
}

HRESULT CSampleGrabberAllocator::Alloc()
{
    CAutoLock lck(this);

    // Check he has called SetProperties
    HRESULT hr = CBaseAllocator::Alloc();
    AVG_ASSERT(!FAILED(hr));

    // If the requirements haven't changed then don't reallocate
    if (hr == S_FALSE) {
        ASSERT(m_pBuffer);
        return NOERROR;
    }
    ASSERT(hr == S_OK); // we use this fact in the loop below

    // Free the old resources
    if (m_pBuffer) {
        ReallyFree();
    }

    // Compute the aligned size
    LONG lAlignedSize = m_lSize + m_lPrefix;
    if (m_lAlignment > 1) {
        LONG lRemainder = lAlignedSize % m_lAlignment;
        if (lRemainder != 0) {
            lAlignedSize += (m_lAlignment - lRemainder);
        }
    }

    // Create the contiguous memory block for the samples
    // making sure it's properly aligned (64K should be enough!)
    AVG_ASSERT(lAlignedSize % m_lAlignment == 0);

    // don't create the buffer - use what was passed to us
    //
    m_pBuffer = m_pPin->m_pBuffer;
    AVG_ASSERT(m_pBuffer);

    LPBYTE pNext = m_pBuffer;
    CMediaSample *pSample;

    AVG_ASSERT(m_lAllocated == 0);

    // Create the new samples - we have allocated m_lSize bytes for each sample
    // plus m_lPrefix bytes per sample as a prefix. We set the pointer to
    // the memory after the prefix - so that GetPointer() will return a pointer
    // to m_lSize bytes.
    for (; m_lAllocated < m_lCount; m_lAllocated++, pNext += lAlignedSize) {
        pSample = new CMediaSample(
                                NAME("Sample Grabber memory media sample"),
                                this,
                                &hr,
                                pNext + m_lPrefix,      // GetPointer() value
                                m_lSize);               // not including prefix

        AVG_ASSERT(SUCCEEDED(hr));
        m_lFree.Add(pSample);
    }
    m_bChanged = FALSE;
    return NOERROR;
}

void CSampleGrabberAllocator::ReallyFree()
{
    AVG_ASSERT(m_lAllocated == m_lFree.GetCount());

    CMediaSample *pSample;
    for (;;) {
        pSample = m_lFree.RemoveHead();
        if (pSample != NULL) {
            delete pSample;
        } else {
            break;
        }
    }
    m_lAllocated = 0;
    // don't free the buffer - let the app do it
}


//----------------------------------------------------------------------------
// SetProperties: Called by the upstream filter to set the allocator
// properties. The application has already allocated the buffer, so we reject 
// anything that is not compatible with that, and return the actual props.
//----------------------------------------------------------------------------

HRESULT CSampleGrabberAllocator::SetProperties(
    ALLOCATOR_PROPERTIES *pRequest, 
    ALLOCATOR_PROPERTIES *pActual
)
{
    HRESULT hr = CMemAllocator::SetProperties(pRequest, pActual);
    if (FAILED(hr)) {
        return hr;
    }
    
    ALLOCATOR_PROPERTIES *pRequired = &(m_pPin->m_allocprops);
    if (pRequest->cbAlign != pRequired->cbAlign) {
        return VFW_E_BADALIGN;
    }
    if (pRequest->cbPrefix != pRequired->cbPrefix) {
        return E_FAIL;
    }
    if (pRequest->cbBuffer > pRequired->cbBuffer) {
        return E_FAIL;
    }
    if (pRequest->cBuffers > pRequired->cBuffers) {
        return E_FAIL;
    }

    *pActual = *pRequired;

    m_lCount = pRequired->cBuffers;
    m_lSize = pRequired->cbBuffer;
    m_lAlignment = pRequired->cbAlign;
    m_lPrefix = pRequired->cbPrefix;

    return S_OK;
}

};