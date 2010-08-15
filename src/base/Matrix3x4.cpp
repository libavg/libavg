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

#include "Matrix3x4.h"
#include "MathHelper.h"

#include <math.h>

using namespace std;

namespace avg {

Matrix3x4::Matrix3x4()
{
    setIdent();
}

Matrix3x4::Matrix3x4(const float *v)
{
    for (int x=0; x<3; ++x) {
        for (int y=0; y<4; ++y) {
            val[x][y] = v[x*4+y];
        }
    }
}

Matrix3x4 Matrix3x4::createTranslate(float x, float y, float z)
{
    Matrix3x4 mat;
    mat.val[0][3] = x;
    mat.val[1][3] = y;
    mat.val[2][3] = z;
    return mat;
}

Matrix3x4 Matrix3x4::createScale(float x, float y, float z)
{
    Matrix3x4 mat;
    mat.val[0][0] = x;
    mat.val[1][1] = y;
    mat.val[2][2] = z;
    return mat;
}

const Matrix3x4& Matrix3x4::operator *=(const Matrix3x4& mat)
{
    for (int x=0; x<3; ++x) {
        float t0 = val[x][0] * mat.val[0][0] + val[x][1] * mat.val[1][0] 
                + val[x][2] * mat.val[2][0];
        float t1 = val[x][0] * mat.val[0][1] + val[x][1] * mat.val[1][1] 
                + val[x][2] * mat.val[2][1];
        float t2 = val[x][0] * mat.val[0][2] + val[x][1] * mat.val[1][2] 
                + val[x][2] * mat.val[2][2];
        val[x][3] = val[x][0] * mat.val[0][3] + val[x][1] * mat.val[1][3] 
                + val[x][2] * mat.val[2][3] + val[x][3];
        val[x][0] = t0; 
        val[x][1] = t1; 
        val[x][2] = t2;
    }
    return *this;
}

void Matrix3x4::setIdent()
{
    for (int x=0; x<3; ++x) {
        for (int y=0; y<4; ++y) {
            val[x][y] = 0.0;
        }
    }
    val[0][0] = 1.0;
    val[1][1] = 1.0;
    val[2][2] = 1.0;
}   

bool almostEqual(const Matrix3x4& mat1, const Matrix3x4& mat2)
{
    for (int x=0; x<3; ++x) {
        for (int y=0; y<4; ++y) {
            if (!almostEqual(mat1.val[x][y], mat2.val[x][y])) {
                return false;
            }
        }
    }
    return true;
}

std::ostream& operator<<(ostream& os, const Matrix3x4& mat)
{
    os << "{" << endl;
    for (int y=0; y<4; ++y) {
        os << "  {" << mat.val[0][y] << "," << mat.val[1][y] << "," << mat.val[2][y] << "}" << endl;
    }
    os << "}" << endl;
    return os;
}


}

