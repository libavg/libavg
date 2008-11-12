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

#ifndef _DSHelper_H_
#define _DSHelper_H_

#include "Camera.h"

#include "../graphics/Bitmap.h"

#include <string>

#define _WIN32_DCOM
#include <atlcomcli.h>
#include <windows.h>
#include <dshow.h>
#include <Qedit.h>


static const GUID MEDIASUBTYPE_Y800 = 
    {0x30303859, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

static const GUID MEDIASUBTYPE_Y160 =
    {0x30363159, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}}; 

namespace avg {

HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
void RemoveGraphFromRot(DWORD pdwRegister);

std::string getStringProp(IPropertyBag *pPropBag, LPCOLESTR pszPropName);

PixelFormat mediaSubtypeToPixelFormat(const GUID& Subtype, const std::string DesiredPF);

std::string mediaTypeToString(const GUID& Type);
std::string mediaSubtypeToString(const GUID& Subtype);
std::string mediaFormattypeToString(const GUID & Formattype);

bool isDSFeatureCamControl(CameraFeature Feature);
long getDSFeatureID(CameraFeature Feature);

}

#endif
