//
//  libavg - Media Playback Engine.
//  Copyright (C) 2012-2020 Ulrich von Zadow
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
//  Original author of this file: Thomas Schott <scotty at c-base dot org>
//

#ifndef _COLLISION_DETECTOR_H_
#define _COLLISION_DETECTOR_H_

#define AVG_PLUGIN

#include "../../../src/wrapper/WrapHelper.h"
#include "../../../src/graphics/Bitmap.h"


namespace avg {

class CollisionDetector
{
public:
    CollisionDetector(BitmapPtr bmpA, BitmapPtr bmpB);
    ~CollisionDetector();

    bool detect(glm::vec2 posA, glm::vec2 posB);

private:
    BitmapPtr m_pBmpA;
    BitmapPtr m_pBmpB;
};

}


#endif

