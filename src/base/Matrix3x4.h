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

#ifndef _Matrix3x4_H_
#define _Matrix3x4_H_

#include "../api.h"

#include <iostream>

namespace avg {
   
struct AVG_API Matrix3x4 {
public:
    float val[3][4];

    Matrix3x4();
    Matrix3x4(const float *);
    static Matrix3x4 createTranslate(float x, float y, float z);
    static Matrix3x4 createScale(float x, float y, float z);
    
    const Matrix3x4& operator *=(const Matrix3x4& mat);

private:
    void setIdent();
};

bool almostEqual(const Matrix3x4& mat1, const Matrix3x4& mat2);

std::ostream& operator<<(std::ostream& os, const Matrix3x4& mat);

}

#endif
