//------------------------------------------------------------------------------
// File: Grabber.cpp
//
// Desc: DirectShow sample code - Implementation file for the SampleGrabber
//       example filter
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "DSSampleGrabber.h"
//#include <streams.h>     // Active Movie (includes windows.h)
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.

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


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////
/*
STDAPI DllRegisterServer() 
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer() 
{
    return AMovieDllRegisterServer2(FALSE);
}
*/
//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

//
// CreateInstance
//
// Provide the way for COM to create a CSampleGrabber object
//
CUnknown * WINAPI CSampleGrabber::CreateInstance(LPUNKNOWN punk, HRESULT *phr) 
{
    ASSERT(phr);
    
    // assuming we don't want to modify the data
    CSampleGrabber *pNewObject = new CSampleGrabber(punk, phr, FALSE);

    if(pNewObject == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return pNewObject;   

} // CreateInstance


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

CSampleGrabber::CSampleGrabber( IUnknown * pOuter, HRESULT * phr, BOOL ModifiesData )
                : CTransInPlaceFilter( TEXT("libavg sample grabber"), (IUnknown*) pOuter, 
                                       CLSID_libavgGrabber, phr, (BOOL)ModifiesData )
                , m_callback( NULL )
{
    // this is used to override the input pin with our own   
    m_pInput = (CTransInPlaceInputPin*) new CSampleGrabberInPin( this, phr );
    if( !m_pInput )
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }
    
    // Ensure that the output pin gets created.  This is necessary because our
    // SetDeliveryBuffer() method assumes that the input/output pins are created, but
    // the output pin isn't created until GetPin() is called.  The 
    // CTransInPlaceFilter::GetPin() method will create the output pin, since we
    // have not already created one.
    IPin *pOutput = GetPin(1);
    // The pointer is not AddRef'ed by GetPin(), so don't release it
}


STDMETHODIMP CSampleGrabber::NonDelegatingQueryInterface( REFIID riid, void ** ppv) 
{
    CheckPointer(ppv,E_POINTER);

    if(riid == IID_IlibavgGrabber) {                
        return GetInterface((IlibavgGrabber *) this, ppv);
    }
    else {
        return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


//----------------------------------------------------------------------------
// This is where you force the sample grabber to connect with one type
// or the other. What you do here is crucial to what type of data your
// app will be dealing with in the sample grabber's callback. For instance,
// if you don't enforce right-side-up video in this call, you may not get
// right-side-up video in your callback. It all depends on what you do here.
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::CheckInputType( const CMediaType * pmt )
{
    CheckPointer(pmt,E_POINTER);
    CAutoLock lock( &m_Lock );

    // if the major type is not set, then accept anything

    GUID g = *m_mtAccept.Type( );
    if( g == GUID_NULL )
    {
        return NOERROR;
    }

    // if the major type is set, don't accept anything else

    if( g != *pmt->Type( ) )
    {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

    // subtypes must match, if set. if not set, accept anything

    g = *m_mtAccept.Subtype( );
    if( g == GUID_NULL )
    {
        return NOERROR;
    }
    if( g != *pmt->Subtype( ) )
    {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

    // format types must match, if one is set

    g = *m_mtAccept.FormatType( );
    if( g == GUID_NULL )
    {
        return NOERROR;
    }
    if( g != *pmt->FormatType( ) )
    {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

    // at this point, for this sample code, this is good enough,
    // but you may want to make it more strict

    return NOERROR;
}


//----------------------------------------------------------------------------
// This bit is almost straight out of the base classes.
// We override this so we can handle Transform( )'s error
// result differently.
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::Receive( IMediaSample * pms )
{
    CheckPointer(pms,E_POINTER);

    HRESULT hr;
    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();

    if (pProps->dwStreamId != AM_STREAM_MEDIA) 
    {
        if( m_pOutput->IsConnected() )
            return m_pOutput->Deliver(pms);
        else
            return NOERROR;
    }

    if (UsingDifferentAllocators()) 
    {
        // We have to copy the data.

        pms = Copy(pms);

        if (pms == NULL) 
        {
            return E_UNEXPECTED;
        }
    }

    // have the derived class transform the data
    hr = Transform(pms);

    if (FAILED(hr)) 
    {
        DbgLog((LOG_TRACE, 1, TEXT("Error from TransInPlace")));
        if (UsingDifferentAllocators()) 
        {
            pms->Release();
        }
        return hr;
    }

    if (hr == NOERROR) 
    {
        hr = m_pOutput->Deliver(pms);
    }
    
    // release the output buffer. If the connected pin still needs it,
    // it will have addrefed it itself.
    if (UsingDifferentAllocators()) 
    {
        pms->Release();
    }

    return hr;
}


//----------------------------------------------------------------------------
// Transform
//----------------------------------------------------------------------------

HRESULT CSampleGrabber::Transform ( IMediaSample * pms )
{
    CheckPointer(pms,E_POINTER);
    CAutoLock lock( &m_Lock );

    if( m_callback )
    {
        REFERENCE_TIME StartTime, StopTime;
        pms->GetTime( &StartTime, &StopTime);

        StartTime += m_pInput->CurrentStartTime( );
        StopTime  += m_pInput->CurrentStartTime( );

        BOOL * pTypeChanged = &((CSampleGrabberInPin*) m_pInput)->m_bMediaTypeChanged;

        HRESULT hr = m_callback( pms, &StartTime, &StopTime, *pTypeChanged );

        *pTypeChanged = FALSE; // now that we notified user, we can clear it

        return hr;
    }

    return NOERROR;
}


//----------------------------------------------------------------------------
// SetAcceptedMediaType
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabber::SetAcceptedMediaType( const CMediaType * pmt )
{
    CAutoLock lock( &m_Lock );

    if( !pmt )
    {
        m_mtAccept = CMediaType( );
        return NOERROR;        
    }

    HRESULT hr;
    hr = CopyMediaType( &m_mtAccept, pmt );

    return hr;
}

//----------------------------------------------------------------------------
// GetAcceptedMediaType
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabber::GetConnectedMediaType( CMediaType * pmt )
{
    if( !m_pInput || !m_pInput->IsConnected( ) )
    {
        return VFW_E_NOT_CONNECTED;
    }

    return m_pInput->ConnectionMediaType( pmt );
}


//----------------------------------------------------------------------------
// SetCallback
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabber::SetCallback( SAMPLECALLBACK Callback )
{
    CAutoLock lock( &m_Lock );

    m_callback = Callback;

    return NOERROR;
}


//----------------------------------------------------------------------------
// inform the input pin of the allocator buffer we wish to use. See the
// input pin's SetDeliverBuffer method for comments. 
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabber::SetDeliveryBuffer( ALLOCATOR_PROPERTIES props, BYTE * m_pBuffer )
{
    // have the input/output pins been created?
    if( !InputPin( ) || !OutputPin( ) )
    {
        return E_POINTER;
    }

    // they can't be connected if we're going to be changing delivery buffers
    //
    if( InputPin( )->IsConnected( ) || OutputPin( )->IsConnected( ) )
    {
        return E_INVALIDARG;
    }

    return ((CSampleGrabberInPin*)m_pInput)->SetDeliveryBuffer( props, m_pBuffer );
}


//----------------------------------------------------------------------------
// used to help speed input pin connection times. We return a partially
// specified media type - only the main type is specified. If we return
// anything BUT a major type, some codecs written improperly will crash
//----------------------------------------------------------------------------

HRESULT CSampleGrabberInPin::GetMediaType( int iPosition, CMediaType * pMediaType )
{
    CheckPointer(pMediaType,E_POINTER);

    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pMediaType = CMediaType( );
    pMediaType->SetType( ((CSampleGrabber*)m_pFilter)->m_mtAccept.Type( ) );

    return S_OK;
}


//----------------------------------------------------------------------------
// override the CTransInPlaceInputPin's method, and return a new enumerator
// if the input pin is disconnected. This will allow GetMediaType to be
// called. If we didn't do this, EnumMediaTypes returns a failure code
// and GetMediaType is never called. 
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabberInPin::EnumMediaTypes( IEnumMediaTypes **ppEnum )
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumMediaTypes *));

    // if the output pin isn't connected yet, offer the possibly 
    // partially specified media type that has been set by the user

    if( !((CSampleGrabber*)m_pTIPFilter)->OutputPin( )->IsConnected() )
    {
        // Create a new reference counted enumerator

        *ppEnum = new CEnumMediaTypes( this, NULL );

        return (*ppEnum) ? NOERROR : E_OUTOFMEMORY;
    }

    // if the output pin is connected, offer it's fully qualified media type

    return ((CSampleGrabber*)m_pTIPFilter)->OutputPin( )->GetConnected()->EnumMediaTypes( ppEnum );
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabberInPin::NotifyAllocator( IMemAllocator *pAllocator, BOOL bReadOnly )
{
    if( m_pPrivateAllocator )
    {
        if( pAllocator != m_pPrivateAllocator )
        {
            return E_FAIL;
        }
        else
        {
            // if the upstream guy wants to be read only and we don't, then that's bad
            // if the upstream guy doesn't request read only, but we do, that's okay
            if( bReadOnly && !SampleGrabber( )->IsReadOnly( ) )
            {
                return E_FAIL;
            }
        }
    }

    return CTransInPlaceInputPin::NotifyAllocator( pAllocator, bReadOnly );
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

STDMETHODIMP CSampleGrabberInPin::GetAllocator( IMemAllocator **ppAllocator )
{
    if( m_pPrivateAllocator )
    {
        CheckPointer(ppAllocator,E_POINTER);

        *ppAllocator = m_pPrivateAllocator;
        m_pPrivateAllocator->AddRef( );
        return NOERROR;
    }
    else
    {
        return CTransInPlaceInputPin::GetAllocator( ppAllocator );
    }
}

//----------------------------------------------------------------------------
// GetAllocatorRequirements: The upstream filter calls this to get our
// filter's allocator requirements. If the app has set the buffer, then
// we return those props. Otherwise, we use the default TransInPlace behavior.
//----------------------------------------------------------------------------

HRESULT CSampleGrabberInPin::GetAllocatorRequirements( ALLOCATOR_PROPERTIES *pProps )
{
    CheckPointer(pProps,E_POINTER);

    if (m_pPrivateAllocator)
    {
        *pProps = m_allocprops;
        return S_OK;
    }
    else
    {
        return CTransInPlaceInputPin::GetAllocatorRequirements(pProps);
    }
}




//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HRESULT CSampleGrabberInPin::SetDeliveryBuffer( ALLOCATOR_PROPERTIES props, BYTE * pBuffer )
{
    // don't allow more than one buffer

    if( props.cBuffers != 1 )
    {
        return E_INVALIDARG;
    }
    if( !pBuffer )
    {
        return E_POINTER;
    }

    m_allocprops = props;
    m_pBuffer = pBuffer;

    // If there is an existing allocator, make sure that it is released
    // to prevent a memory leak
    if (m_pPrivateAllocator)
    {
        m_pPrivateAllocator->Release();
        m_pPrivateAllocator = NULL;
    }

    HRESULT hr = S_OK;

    m_pPrivateAllocator = new CSampleGrabberAllocator( this, &hr );
    if( !m_pPrivateAllocator )
    {
        return E_OUTOFMEMORY;
    }

    m_pPrivateAllocator->AddRef( );
    return hr;
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HRESULT CSampleGrabberInPin::SetMediaType( const CMediaType *pmt )
{
    m_bMediaTypeChanged = TRUE;

    return CTransInPlaceInputPin::SetMediaType( pmt );
}


//----------------------------------------------------------------------------
// don't allocate the memory, just use the buffer the app provided
//----------------------------------------------------------------------------

HRESULT CSampleGrabberAllocator::Alloc( )
{
    // look at the base class code to see where this came from!

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
    if (m_lAlignment > 1) 
    {
        LONG lRemainder = lAlignedSize % m_lAlignment;
        if (lRemainder != 0) 
        {
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
    for (; m_lAllocated < m_lCount; m_lAllocated++, pNext += lAlignedSize) 
    {
        pSample = new CMediaSample(
                                NAME("Sample Grabber memory media sample"),
                                this,
                                &hr,
                                pNext + m_lPrefix,      // GetPointer() value
                                m_lSize);               // not including prefix

        ASSERT(SUCCEEDED(hr));
        if (pSample == NULL)
            return E_OUTOFMEMORY;

        // This CANNOT fail
        m_lFree.Add(pSample);
    }

    m_bChanged = FALSE;
    return NOERROR;
}


//----------------------------------------------------------------------------
// don't really free the memory
//----------------------------------------------------------------------------

void CSampleGrabberAllocator::ReallyFree()
{
    // look at the base class code to see where this came from!

    // Should never be deleting this unless all buffers are freed

    ASSERT(m_lAllocated == m_lFree.GetCount());

    // Free up all the CMediaSamples

    CMediaSample *pSample;
    for (;;) 
    {
        pSample = m_lFree.RemoveHead();
        if (pSample != NULL) 
        {
            delete pSample;
        } 
        else 
        {
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

    if (FAILED(hr))
    {
        return hr;
    }
    
    ALLOCATOR_PROPERTIES *pRequired = &(m_pPin->m_allocprops);
    if (pRequest->cbAlign != pRequired->cbAlign)
    {
        return VFW_E_BADALIGN;
    }
    if (pRequest->cbPrefix != pRequired->cbPrefix)
    {
        return E_FAIL;
    }
    if (pRequest->cbBuffer > pRequired->cbBuffer)
    {
        return E_FAIL;
    }
    if (pRequest->cBuffers > pRequired->cBuffers)
    {
        return E_FAIL;
    }

    *pActual = *pRequired;

    m_lCount = pRequired->cBuffers;
    m_lSize = pRequired->cbBuffer;
    m_lAlignment = pRequired->cbAlign;
    m_lPrefix = pRequired->cbPrefix;

    return S_OK;
}
