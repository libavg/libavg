//
// $Id$
// 

#include "AVGSDLFontManager.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGSDLFont.h"

#include <SDL/SDL_ttf.h>

#include <iostream>
#include <sstream>
#include <stdlib.h>

using namespace std;

AVGSDLFontManager::AVGSDLFontManager (const string& sFontPath)
    : AVGFontManager(sFontPath)
{
//    if (!TTF_WasInit()) {
        int err = TTF_Init();
        if (err == -1) {
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    "Could not initialize SDL_ttf. Font support is broken.");
        }
//    }
}

AVGSDLFontManager::~AVGSDLFontManager ()
{
    TTF_Quit();
}

IAVGFont * AVGSDLFontManager::loadFont(const string& Filename, int Size)
{
    return new AVGSDLFont(Filename, Size);
}

