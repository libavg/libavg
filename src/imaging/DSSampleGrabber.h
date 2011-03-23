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

#ifndef _DSSampleGrabber_H_
#define _DSSampleGrabber_H_

#include <Guiddef.h>
#define _WIN32_DCOM
#include <windows.h>
#include <dshow.h>
#include <dshowbase/mtype.h>
#include <dshowbase/wxdebug.h>
#include <dshowbase/combase.h>
#include <dshowbase/reftime.h>
#include <dshowbase/wxutil.h>
#include <dshowbase/wxlist.h>
#include <dshowbase/amfilter.h>
#include <dshowbase/transfrm.h>
#include <dshowbase/transip.h>

// {455A53B7-FC34-4960-94CE-A17A0B23F807}
DEFINE_GUID(CLSID_libavgGrabber, 
0x455a53b7, 0xfc34, 0x4960, 0x94, 0xce, 0xa1, 0x7a, 0xb, 0x23, 0xf8, 0x7);

// {87F09DC5-12BC-479d-A20F-21133C613037}
DEFINE_GUID(IID_IlibavgGrabber, 
0x87f09dc5, 0x12bc, 0x479d, 0xa2, 0xf, 0x21, 0x13, 0x3c, 0x61, 0x30, 0x37);

class ISampleCallback 
{
public:
    virtual void onSample(IMediaSample * pSample)=0;
};

MIDL_INTERFACE("6B652FFF-11FE-4FCE-92AD-0266B5D7C78F")
IlibavgGrabber : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(CMediaType* pType) = 0;
    virtual void STDMETHODCALLTYPE SetCallback(ISampleCallback* pCallback) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDeliveryBuffer(ALLOCATOR_PROPERTIES props,
            BYTE *pBuffer) = 0;
};
        
class CSampleGrabberInPin;
class CSampleGrabber;

class CSampleGrabberAllocator : public CMemAllocator
{
public:
    CSampleGrabberAllocator(CSampleGrabberInPin* pParent, HRESULT* phr) 
        : CMemAllocator(TEXT("SampleGrabberAllocator\0"), NULL, phr),
          m_pPin(pParent)
    {
    };

    ~CSampleGrabberAllocator( )
    {
        m_pBuffer = NULL;
    }

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
    CSampleGrabberInPin(CTransInPlaceFilter* pFilter, HRESULT* pHr ) 
        : CTransInPlaceInputPin(TEXT("SampleGrabberInputPin\0"), pFilter, pHr, 
                L"Input\0"),
          m_pPrivateAllocator(NULL),
          m_pBuffer(NULL),
          m_bMediaTypeChanged(FALSE)
    {
        memset(&m_allocprops, 0, sizeof(m_allocprops));
    }

    ~CSampleGrabberInPin( )
    {
        if (m_pPrivateAllocator) {
            delete m_pPrivateAllocator;
        }
    }

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
    CSampleGrabber * SampleGrabber() { return (CSampleGrabber*) m_pFilter; }
    HRESULT SetDeliveryBuffer(ALLOCATOR_PROPERTIES props, BYTE* m_pBuffer);

private:
    friend class CSampleGrabberAllocator;
    friend class CSampleGrabber;

    CSampleGrabberAllocator* m_pPrivateAllocator;
    ALLOCATOR_PROPERTIES m_allocprops;
    BYTE* m_pBuffer;
    BOOL m_bMediaTypeChanged;
};


class CSampleGrabber : public CTransInPlaceFilter, IlibavgGrabber
{
public:
    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
    CSampleGrabber(IUnknown* pOuter, HRESULT* pHr);

    // Expose ISampleGrabber
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
    DECLARE_IUNKNOWN;

    // IlibavgGrabber
    STDMETHODIMP GetConnectedMediaType(CMediaType * pmt);
    void STDMETHODCALLTYPE SetCallback(ISampleCallback* pCallback);
    STDMETHODIMP SetDeliveryBuffer(ALLOCATOR_PROPERTIES props, BYTE * m_pBuffer);

protected:
    ISampleCallback* m_pCallback;
    CCritSec m_Lock; // serialize access to our data

    HRESULT CheckInputType(const CMediaType* pmt);
    HRESULT Transform(IMediaSample* pms);

    // override this so we can return S_FALSE directly. 
    // The base class CTransInPlace
    // Transform( ) method is called by it's 
    // Receive( ) method. There is no way
    // to get Transform( ) to return an S_FALSE value 
    // (which means "stop giving me data"),
    // to Receive( ) and get Receive( ) to return S_FALSE as well.
    HRESULT Receive(IMediaSample* pms);

private:
    friend class CSampleGrabberInPin;
    friend class CSampleGrabberAllocator;
};

#endif