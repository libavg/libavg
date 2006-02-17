//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

// The code in this file is derived from code at 
// http://developer.apple.com/qa/qa2001/qa1188.html

#include "AppleGLHelper.h"

#include "../base/Logger.h"

CFBundleRef gBundleRefOpenGL = NULL;

using namespace avg;

OSStatus aglInitEntryPoints (void)
{
    OSStatus err = noErr;
    Str255 frameworkName;
    CopyCStringToPascal("OpenGL.framework", frameworkName);
    FSRefParam fileRefParam;
    FSRef fileRef;
    CFURLRef bundleURLOpenGL;

    memset(&fileRefParam, 0, sizeof(fileRefParam));
    memset(&fileRef, 0, sizeof(fileRef));

    fileRefParam.ioNamePtr  = frameworkName;
    fileRefParam.newRef = &fileRef;

    // Frameworks directory/folder
    err = FindFolder (kSystemDomain, kFrameworksFolderType, false,
                      &fileRefParam.ioVRefNum, &fileRefParam.ioDirID);
    if (noErr != err) {
        AVG_TRACE(Logger::WARNING, "Could not find frameworks folder");
        return err;
    }
    err = PBMakeFSRefSync (&fileRefParam); // make FSRef for folder
    if (noErr != err) {
        AVG_TRACE (Logger::WARNING, "Could not make FSref to frameworks folder");
        return err;
    }
    // create URL to folder
    bundleURLOpenGL = CFURLCreateFromFSRef (kCFAllocatorDefault,
                                            &fileRef);
    if (!bundleURLOpenGL) {
        AVG_TRACE (Logger::WARNING, "Could not create OpenGL Framework bundle URL");
        return paramErr;
    }
    // create ref to GL's bundle
    gBundleRefOpenGL = CFBundleCreate (kCFAllocatorDefault,
                                       bundleURLOpenGL);
    if (!gBundleRefOpenGL) {
        AVG_TRACE (Logger::WARNING, "Could not create OpenGL Framework bundle");
        return paramErr;
    }
    CFRelease (bundleURLOpenGL); // release created bundle
    // if the code was successfully loaded, look for our function.
    if (!CFBundleLoadExecutable (gBundleRefOpenGL)) {
        AVG_TRACE (Logger::WARNING, "Could not load MachO executable");
        return paramErr;
    }
    return err;
}

// -------------------------

void aglDellocEntryPoints (void)
{
    if (gBundleRefOpenGL != NULL) {
        // unload the bundle's code.
        CFBundleUnloadExecutable (gBundleRefOpenGL);
        CFRelease (gBundleRefOpenGL);
        gBundleRefOpenGL = NULL;
    }
}

// -------------------------

void * aglGetProcAddress (const char * pszProc)
{
    return CFBundleGetFunctionPointerForName (gBundleRefOpenGL,
                CFStringCreateWithCStringNoCopy (NULL,
                     pszProc, CFStringGetSystemEncoding (), NULL));
}

