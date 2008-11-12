// Copyright (C) 2008 Archimedes Solutions GmbH,
// Saarbrücker Str. 24b, Berlin, Germany
//
// This file contains proprietary source code and confidential
// information. Its contents may not be disclosed or distributed to
// third parties unless prior specific permission by Archimedes
// Solutions GmbH, Berlin, Germany is obtained in writing. This applies
// to copies made in any form and using any medium. It applies to
// partial as well as complete copies.

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
