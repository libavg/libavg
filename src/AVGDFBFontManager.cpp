//
// $Id$
// 

#include "AVGDFBFontManager.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGDFBFont.h"

#include <iostream>
#include <sstream>
#include <stdlib.h>

using namespace std;

AVGDFBFontManager::AVGDFBFontManager (IDirectFB * pDFB, const string& sFontPath)
    : m_pDFB(pDFB), 
      AVGFontManager (sFontPath)
{
}

AVGDFBFontManager::~AVGDFBFontManager ()
{
}

IAVGFont * AVGDFBFontManager::loadFont(const string& Filename, int Size)
{
    return new AVGDFBFont(m_pDFB, Filename, Size);
}

