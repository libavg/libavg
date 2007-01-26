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
//  Original author of this file is igor@c-base.org 
//

#ifndef _ConnectedComps_H_
#define _ConnectedComps_H_

#include "BlobInfo.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"
#include "../graphics/Pixel32.h"

#include <boost/shared_ptr.hpp>

#include <assert.h>
#include <list>
#include <vector>
#include <map>

namespace avg {

struct Run
{
        Run(int row, int start_col, int end_col, int color);
        int m_Row;
        int m_StartCol;
        int m_EndCol;
        int m_Color;
        int length();
        DPoint center();
        int m_Label;
    private:
        static int s_LastLabel;
};

typedef boost::shared_ptr<struct Run> RunPtr;
typedef std::vector<struct Run> RunList;

class Blob;
typedef boost::shared_ptr<class Blob> BlobPtr;

class Blob
{
    public:
        Blob(Run run);
        ~Blob();
        RunList& get_runs();
        DPoint center();
        int area();
        int getLabel();
        BlobInfoPtr getInfo();
        IntRect bbox();
        void merge( BlobPtr other);
        RunList* getList();
        void render(Bitmap *pTarget, Pixel32 Color, bool bMarkCenter, 
                Pixel32 CenterColor= Pixel32(0x00, 0x00, 0xFF, 0xFF));
        BlobPtr m_pParent;
    private:
        Blob(const Blob &);
        RunList *m_pRuns;
};

typedef std::vector<BlobPtr> BlobList;
typedef boost::shared_ptr<BlobList> BlobListPtr;
typedef std::map<int, BlobPtr> CompsMap;

BlobListPtr connected_components(BitmapPtr image, unsigned char object_threshold);
BlobListPtr connected_components(BitmapPtr image, BitmapPtr thresholds);

}

#endif
