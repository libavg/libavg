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

#include "Bitmap.h"

#include "../base/Test.h"

#include <string>

namespace avg {

class GraphicsTest: public Test {
public:
    GraphicsTest(const std::string& sName, int indentLevel);
    
    static void createResultImgDir();

protected:
    BitmapPtr loadTestBmp(const std::string& sFName, PixelFormat pf = NO_PIXELFORMAT);
    void testEqual(Bitmap& ResultBmp, const std::string& sFName, 
            PixelFormat pf = NO_PIXELFORMAT); 
    void testEqual(Bitmap& ResultBmp, Bitmap& BaselineBmp,
        const std::string& sFName);
    void testEqualBrightness(Bitmap& ResultBmp, Bitmap& BaselineBmp, int epsilon);

private:
    int sumPixels(Bitmap& Bmp);
    bool bmpAlmostEqual(Bitmap& Bmp1, Bitmap& Bmp2);
};


}
