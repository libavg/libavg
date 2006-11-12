#include "ConnectedComps.h"

#include "../graphics/Filtergrayscale.h"

#include <iostream>

using namespace avg;

int main(int argc, char **argv) {
    char *fname = argv[1];
    avg::BitmapPtr tmp_im = avg::BitmapPtr(new avg::Bitmap(fname));
    avg::FilterGrayscale grayscale = avg::FilterGrayscale(); 
    avg::BitmapPtr im = grayscale.apply(tmp_im);


    avg::DPoint center;
    int area;
    avg::DPointList *ev;
    double stddev;
    for(int i=0;i<20;i++){
        avg::BlobList *bloblist = avg::connected_components(im,100);
        for(avg::BlobList::iterator b=bloblist->begin();b!=bloblist->end();b++){
            center = (*b)->center();
            area = (*b)->area();
            ev = (*b)->pca();
            std::cout<<"center = ("<<center.x<<","<<center.y<<")"<<std::endl;
            std::cout<<"area = "<<area<<std::endl;
            for (avg::DPointList::iterator e = ev->begin();e!=ev->end();e++){
                std::cout<<"["<<(*e).x<<","<<(*e).y<<"]"<<std::endl;
            }
            //std::cout<<"stddev = "<<stddev<<std::endl;
            std::cout<<"==============================="<<std::endl;
            ev->clear();
            delete ev;
        }
//        std::cout<<i<<std::endl;
        delete bloblist;
    }
}
