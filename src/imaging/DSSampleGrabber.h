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

#ifndef _DSSampleGrabber_H_
#define _DSSampleGrabber_H_

#include "IDSSampleCallback.h"

#include "../base/Exception.h"

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

// {87F09DC5-12BC-479d-A20F-21133C613037}
DEFINE_GUID(IID_IlibavgGrabber, 
0x87f09dc5, 0x12bc, 0x479d, 0xa2, 0xf, 0x21, 0x13, 0x3c, 0x61, 0x30, 0x37);

namespace avg {

MIDL_INTERFACE("6B652FFF-11FE-4FCE-92AD-0266B5D7C78F")
IlibavgGrabber : public IUnknown
{
public:
    virtual void STDMETHODCALLTYPE SetCallback(IDSSampleCallback* pCallback) = 0;
};
        
class CSampleGrabberInPin;

class CSampleGrabber : public CTransInPlaceFilter, IlibavgGrabber
{
public:
    CSampleGrabber(IUnknown* pOuter, HRESULT* pHr);

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
    DECLARE_IUNKNOWN;

    // IlibavgGrabber
    void STDMETHODCALLTYPE SetCallback(IDSSampleCallback* pCallback);

protected:
    IDSSampleCallback* m_pCallback;
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
};

};

#endif