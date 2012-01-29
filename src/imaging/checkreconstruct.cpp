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

#include "../graphics/Filtergrayscale.h"

#include "Reconstruct.h"

using namespace avg;
using namespace std;


void filterImage(const string& fName)
{
    BitmapPtr pSrcBmp(new Bitmap("testfiles/"+fName+".png"));
    BitmapPtr pFreqBmp;
    FilterGrayscale().applyInPlace(pSrcBmp);
    pSrcBmp->save("resultimages/"+fName+"_orig.png");
    BitmapPtr pDestBmp = lowpass(pSrcBmp, pFreqBmp, 0.25);
    pFreqBmp->save("resultimages/"+fName+"_freq.png");
    pDestBmp->save("resultimages/"+fName+".png");
}

int main(int nargs, char** args)
{
    filterImage("spike");
    filterImage("spike_rect");
    filterImage("rgb24-64x64");
}

