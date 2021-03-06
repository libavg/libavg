//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2021 Ulrich von Zadow
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

#include "../api.h"
#include "Bitmap.h"

#include "../base/Test.h"

#include <string>

namespace avg {

class AVG_API GraphicsTest: public Test {
public:
    GraphicsTest(const std::string& sName, int indentLevel);
    
    static void createResultImgDir();

protected:
    std::string getTestBmpName(const std::string& sFName);
    BitmapPtr loadTestBmp(const std::string& sFName, PixelFormat pf = NO_PIXELFORMAT);
    virtual void testEqual(Bitmap& resultBmp, const std::string& sFName, 
            PixelFormat pf = NO_PIXELFORMAT, float maxAverage=0.01f, 
            float maxStdDev=0.05f); 
    virtual void testEqual(Bitmap& resultBmp, Bitmap& baselineBmp,
        const std::string& sFName, float maxAverage=0.01f, float maxStdDev=0.05f);
    void testEqualBrightness(Bitmap& resultBmp, Bitmap& baselineBmp, float epsilon);

private:
    int sumPixels(Bitmap& bmp);

};


}
