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

#ifndef _CoordTransformer_H_
#define _CoordTransformer_H_

#include "../graphics/Point.h"
#include "../graphics/Rect.h"

namespace avg {

class CoordTransformer {
    public:
        CoordTransformer();
        virtual ~CoordTransformer();

        void load(const std::string & sFilename);
        void save(const std::string & sFilename);

        DPoint transform(const DPoint & pt); //(x,y) -> (x', y')
        DPoint getPixelSize(const DPoint & pt); //A(x,y) -> A'(x',y')

    private:
        IntRect m_SrcRect;
        IntRect m_DestRect;
        double m_TrapezoidFactor;
        //pincushion/barrel correction
        //K1<0 correct barrel
        //K1>0 correct pincushion
        //see http://www.imatest.com/docs/distortion.html
        double m_K1; 

        //DPoint **m_pCache; // m_pCache[x][y] - (new_x, new_y)
};

}
