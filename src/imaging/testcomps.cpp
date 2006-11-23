#include "ConnectedComps.h"

#include "../graphics/Filtergrayscale.h"

#include <iostream>
#include <math.h>
using namespace avg;

int main(int argc, char **argv) {
    char *fname = argv[1];
    avg::BitmapPtr tmp_im = avg::BitmapPtr(new avg::Bitmap(fname));
    avg::FilterGrayscale grayscale = avg::FilterGrayscale(); 
    avg::BitmapPtr im = grayscale.apply(tmp_im);


    avg::DPoint center;
    int area;
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
