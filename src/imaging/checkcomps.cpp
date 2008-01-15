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

#include "Blob.h"

#include "../graphics/Filtergrayscale.h"

#include <iostream>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace avg;
using namespace std;

int main(int argc, char **argv)
{
    char *fname = argv[1];
    BitmapPtr tmp_im = BitmapPtr(new Bitmap(fname));
    FilterGrayscale grayscale = FilterGrayscale(); 
    BitmapPtr im = grayscale.apply(tmp_im);

    DPoint center;
    for(int i=0;i<20;i++){
        BlobVectorPtr bloblist = connected_components(im, 100);
        for(BlobVector::iterator b=bloblist->begin();b!=bloblist->end();b++){
            cout<<"center = ("<<(*b)->getCenter().x<<","<<(*b)->getCenter().y<<")"<<endl;
            cout<<"area = "<<(*b)->getArea()<<endl;
            cout<<"orientation = "<<360*((*b)->getOrientation()/(2*M_PI))<<endl;
            cout<<"eccentricity = "<<(*b)->getEccentricity()<<endl;
            cout<<"["<<((*b)->getScaledBasis(0)).x<<","
                    <<((*b)->getScaledBasis(0)).y<<"]"<<endl;
            cout<<"["<<((*b)->getScaledBasis(1)).x<<","
                    <<((*b)->getScaledBasis(1)).y<<"]"<<endl;
            //cout<<"stddev = "<<stddev<<endl;
            cout<<"==============================="<<endl;
        }
//        cout<<i<<endl;
        bloblist->clear();
    }
}
