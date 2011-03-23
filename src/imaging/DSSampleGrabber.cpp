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
#include <initguid.h>

#include "qedit.h"

#pragma warning(disable: 4800)

const AMOVIESETUP_PIN psudSampleGrabberPins[] =
{ { L"Input"            // strName
  , FALSE               // bRendered
  , FALSE               // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L""                 // strConnectsToPin
  , 0                   // nTypes
  , NULL                // lpTypes
  }
, { L"Output"           // strName
  , FALSE               // bRendered
  , TRUE                // bOutput
  , FALSE               // bZero
  , FALSE               // bMany
  , &CLSID_NULL         // clsConnectsToFilter
  , L""                 // strConnectsToPin
  , 0                   // nTypes
  , NULL                // lpTypes
  }
};

const AMOVIESETUP_FILTER sudSampleGrabber =
{ &CLSID_libavgGrabber            // clsID
, L"libavg sample grabber"        // strName
, MERIT_DO_NOT_USE                // dwMerit
, 2                               // nPins
, psudSampleGrabberPins };        // lpPin


// Needed for the CreateInstance mechanism
CFactoryTemplate g_Templates[]=
{
    { L"libavg sample grabber"
        , &CLSID_libavgGrabber
        , CSampleGrabber::CreateInstance
        , NULL
        , &sudSampleGrabber }

};

int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

CUnknown * WINAPI CSampleGrabber::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {
    ASSERT(phr);
    
    CSampleGrabber *pNewObject = new CSampleGrabber(punk, phr);

    if(pNewObject == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return pNewObject;   

} // CreateInstance

CSampleGrabber::CSampleGrabber(IUnknown * pOuter, HRESULT * phr)
    : CTransInPlaceFilter(TEXT("libavg sample grabber"), (IUnknown*) pOuter, 
              CLSID_libavgGrabber, phr, false),
      m_pCallback(NULL)
{
    m_pInput = (CTransInPlaceInputPin*) new CSampleGrabberInPin(this, phr);
    
    IPin *pOutput = GetPin(1);
}

STDMETHODIMP CSampleGrabber::NonDelegatingQueryInterface(REFIID riid, void** ppv) {
    CheckPointer(ppv, E_POINTER);

    if (riid == IID_IlibavgGrabber) {                
        return GetInterface((IlibavgGrabber *)this, ppv);
    } else {
        return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

HRESULT CSampleGrabber::CheckInputType(const CMediaType* pmt) {
    CheckPointer(pmt, E_POINTER);
    CAutoLock lock(&m_Lock);

    if(MEDIATYPE_Video == *pmt->Type( )) {
        return NOERROR;
    } else {
        return VFW_E_INVALID_MEDIA_TYPE;
    }
}

HRESULT CSampleGrabber::Receive(IMediaSample * pms) {
    CheckPointer(pms, E_POINTER);

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
        if (pms == NULL) {
            return E_UNEXPECTED;
        }
    }

    hr = Transform(pms);
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, 1, TEXT("Error from TransInPlace")));
        if (UsingDifferentAllocators()) {
            pms->Release();
        }
        return hr;
    }
    if (hr == NOERROR) {
        hr = m_pOutput->Deliver(pms);
    }
    if (UsingDifferentAllocators()) {
        pms->Release();
    }
    return hr;
}

HRESULT CSampleGrabber::Transform (IMediaSample * pms) {
    CheckPointer(pms, E_POINTER);
    CAutoLock lock(&m_Lock);

    if (m_pCallback) {
        REFERENCE_TIME StartTime, StopTime;
        pms->GetTime( &StartTime, &StopTime);

        StartTime += m_pInput->CurrentStartTime();
        StopTime  += m_pInput->CurrentStartTime();

        m_pCallback->onSample(pms);
    }
    return NOERROR;
}

STDMETHODIMP CSampleGrabber::GetConnectedMediaType(CMediaType* pmt) {
    if (!m_pInput || !m_pInput->IsConnected( )) {
        return VFW_E_NOT_CONNECTED;
    }

    return m_pInput->ConnectionMediaType(pmt);
}


void STDMETHODCALLTYPE CSampleGrabber::SetCallback(ISampleCallback* pCallback) {
    CAutoLock lock( &m_Lock );
    m_pCallback = pCallback;
}

STDMETHODIMP CSampleGrabber::SetDeliveryBuffer(ALLOCATOR_PROPERTIES props, 
        BYTE* m_pBuffer)
{
    if (!InputPin() || !OutputPin()) {
        return E_POINTER;
    }

    if (InputPin()->IsConnected() || OutputPin()->IsConnected()) {
        return E_INVALIDARG;
    }

    return ((CSampleGrabberInPin*)m_pInput)->SetDeliveryBuffer(props, m_pBuffer);
}

HRESULT CSampleGrabberInPin::GetMediaType(int iPosition, CMediaType* pMediaType) {
    CheckPointer(pMediaType,E_POINTER);

    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pMediaType = CMediaType( );
    pMediaType->SetType(&MEDIATYPE_Video);

    return S_OK;
}

STDMETHODIMP CSampleGrabberInPin::EnumMediaTypes(IEnumMediaTypes** ppEnum)
{
    CheckPointer(ppEnum, E_POINTER);
    ValidateReadWritePtr(ppEnum, sizeof(IEnumMediaTypes *));

    if(!((CSampleGrabber*)m_pTIPFilter)->OutputPin( )->IsConnected())
    {
        *ppEnum = new CEnumMediaTypes( this, NULL );
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


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabberInPin::GetAllocator(IMemAllocator** ppAllocator) {
    if (m_pPrivateAllocator) {
        CheckPointer(ppAllocator,E_POINTER);

        *ppAllocator = m_pPrivateAllocator;
        m_pPrivateAllocator->AddRef();
        return NOERROR;
    } else {
        return CTransInPlaceInputPin::GetAllocator(ppAllocator);
    }
}

HRESULT CSampleGrabberInPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    CheckPointer(pProps,E_POINTER);

    if (m_pPrivateAllocator) {
        *pProps = m_allocprops;
        return S_OK;
    } else {
        return CTransInPlaceInputPin::GetAllocatorRequirements(pProps);
    }
}

HRESULT CSampleGrabberInPin::SetDeliveryBuffer(ALLOCATOR_PROPERTIES props, BYTE* pBuffer)
{
    if (props.cBuffers != 1) {
        return E_INVALIDARG;
    }
    if (!pBuffer) {
        return E_POINTER;
    }

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

HRESULT CSampleGrabberInPin::SetMediaType(const CMediaType *pmt) {
    m_bMediaTypeChanged = TRUE;
    return CTransInPlaceInputPin::SetMediaType(pmt);
}

HRESULT CSampleGrabberAllocator::Alloc() {
    CAutoLock lck(this);

    // Check he has called SetProperties
    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
        return hr;
    }

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
    ASSERT(lAlignedSize % m_lAlignment == 0);

    // don't create the buffer - use what was passed to us
    //
    m_pBuffer = m_pPin->m_pBuffer;

    if (m_pBuffer == NULL) {
        return E_OUTOFMEMORY;
    }

    LPBYTE pNext = m_pBuffer;
    CMediaSample *pSample;

    ASSERT(m_lAllocated == 0);

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

        ASSERT(SUCCEEDED(hr));
        m_lFree.Add(pSample);
    }
    m_bChanged = FALSE;
    return NOERROR;
}

void CSampleGrabberAllocator::ReallyFree()
{
    ASSERT(m_lAllocated == m_lFree.GetCount());

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
