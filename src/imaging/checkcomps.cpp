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
//  Original author of this file is igor@c-base.org.
//

#include "ConnectedComps.h"

#include "../graphics/Filtergrayscale.h"

#include <iostream>
#include <math.h>

using namespace avg;

int main(int argc, char **argv)
{
    char *fname = argv[1];
    avg::BitmapPtr tmp_im = avg::BitmapPtr(new avg::Bitmap(fname));
    avg::FilterGrayscale grayscale = avg::FilterGrayscale(); 
    avg::BitmapPtr im = grayscale.apply(tmp_im);

    avg::DPoint center;
    avg::BlobInfoPtr info;
    for(int i=0;i<20;i++){
        avg::BlobListPtr bloblist = avg::connected_components(im,100);
        for(avg::BlobList::iterator b=bloblist->begin();b!=bloblist->end();b++){
            info = (*b)->getInfo();
            std::cout<<"center = ("<<info->m_Center.x<<","<<info->m_Center.y<<")"<<std::endl;
            std::cout<<"area = "<<info->m_Area<<std::endl;
            std::cout<<"orientation = "<<360*(info->m_Orientation/(2*M_PI))<<std::endl;
            std::cout<<"eccentricity = "<<info->m_Eccentricity<<std::endl;
            std::cout<<"["<<(info->m_ScaledBasis[0]).x<<","<<(info->m_ScaledBasis[0]).y<<"]"<<std::endl;
            std::cout<<"["<<(info->m_ScaledBasis[1]).x<<","<<(info->m_ScaledBasis[1]).y<<"]"<<std::endl;
            //std::cout<<"stddev = "<<stddev<<std::endl;
            std::cout<<"==============================="<<std::endl;
        }
//        std::cout<<i<<std::endl;
        bloblist->clear();
    }
}
