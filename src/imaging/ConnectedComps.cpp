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

#include "ConnectedComps.h"

#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace avg {

int Run::s_LastLabel= 0;

Run::Run(int row, int start_col, int end_col, int color)
{
    m_Row = row;
    assert(end_col>=start_col);
    m_StartCol = start_col;
    m_EndCol = end_col;
    m_Color = color;
    s_LastLabel++;
    m_Label = s_LastLabel;
}

int Run::length()
{
    return m_EndCol-m_StartCol+1;
}

DPoint Run::center()
{
    DPoint d = DPoint((m_StartCol + m_EndCol)/2., m_Row);
    return d;
}

Blob::Blob(Run run) 
{
    m_pRuns = new RunList();
    m_pRuns->push_back(run);
    m_pParent = BlobPtr();
}

Blob::~Blob() 
{
    delete m_pRuns;
}

RunList *Blob::getList()
{
    return m_pRuns;
}

void Blob::merge(BlobPtr other)
{
    assert(other);
    RunList *other_runs=other->getList();
    for(RunList::iterator it=other_runs->begin();it!=other_runs->end();++it){
        m_pRuns->push_back(*it);
    }
    //m_pRuns->splice(m_pRuns->end(), *(other->getList()));
}

DPoint Blob::center()
{
    DPoint d = DPoint(0,0);
    int c = 0;
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();++r){
        d+=r->center();
        c++;
    }

    return d/double(c);
}
IntRect Blob::bbox()
{
    int x1=__INT_MAX__,y1=__INT_MAX__,x2=0,y2=0;
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();++r){
        x1 = std::min(x1, r->m_StartCol);
        y1 = std::min(y1, r->m_Row);
        x2 = std::max(x2, r->m_EndCol);
        y2 = std::max(y2, r->m_Row);
    }
    return IntRect(x1,y1,x2+1,y2+1);

}

int Blob::area()
{
    int res = 0;
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();++r){
        res+= r->length();
    }
    return res;
}

int Blob::getLabel()
{
    if(!m_pRuns->empty()){
        return (m_pRuns->begin())->m_Label;
    }else{
        return 0;//actually invalid
    }
}

void Blob::render(Bitmap *pTarget, Pixel32 Color, bool bMarkCenter, 
        Pixel32 CenterColor)
{
    assert (pTarget->getBytesPerPixel() == 4);
    unsigned char *ptr;
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();++r){
        ptr = pTarget->getPixels()+r->m_Row*pTarget->getStride();
        int x_pos = r->m_StartCol;
        ptr+= x_pos*4;
        while(x_pos<=r->m_EndCol){
            *((unsigned int *)ptr)=(unsigned int)Color;
            ptr += 4;
            x_pos++;
        }
    }
    if(bMarkCenter) {
        DPoint DCenter = center();
        IntPoint Center = IntPoint(int(DCenter.x+0.5), int(DCenter.y+0.5));
        IntPoint size = pTarget->getSize();
        int xstart = std::max(0,Center.x-5);
        int xstop = std::min(Center.x+5,size.x-1);
        int ystart = std::max(0,Center.y-5);
        int ystop = std::min(Center.y+5,size.y-1);

        ptr = pTarget->getPixels()+Center.y*pTarget->getStride()+xstart*4;
        for(int x=xstart;x<=xstop;++x){
            *((unsigned int *)ptr)=(unsigned int)CenterColor;
            ptr += 4;
        }
        ptr = pTarget->getPixels()+ystart*pTarget->getStride()+Center.x*4;
        for(int y=ystart;y<=ystop;++y){
            *((unsigned int *)ptr)=(unsigned int)CenterColor;
            ptr+=pTarget->getStride();
        }
    }
}

BlobInfoPtr Blob::getInfo()
{
    /*
        more useful numbers that can be calculated from c
        see e.g. 
        <http://www.cs.cf.ac.uk/Dave/Vision_lecture/node36.html#SECTION00173000000000000000>
        
        Orientation = tan−1(2(c_xy)/(c_xx − c_yy)) /2
        Inertia = c_xx + c_yy
        Eccentricity = ...
    */
    double c_xx = 0, c_yy =0, c_xy = 0, ll=0;
    BlobInfoPtr res = BlobInfoPtr(new BlobInfo());
    DPoint c = res->m_Center = center();
    res->m_BoundingBox = bbox();
    double A = res->m_Area = area();
    double l1, l2;
    double tmp_x, tmp_y, mag;
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();++r){
        //This is the evaluated expression for the variance when using runs...
        ll = r->length();
        c_yy += ll* (r->m_Row- c.y)*(r->m_Row- c.y);
        c_xx += ( r->m_EndCol * (r->m_EndCol+1) * (2*r->m_EndCol+1) - (r->m_StartCol-1) * r->m_StartCol * (2*r->m_StartCol -1))/6. - 
            c.x * ( r->m_EndCol*(r->m_EndCol+1) - (r->m_StartCol-1)*r->m_StartCol  )
            + ll* c.x*c.x;
        c_xy += (r->m_Row-c.y)*0.5*( r->m_EndCol*(r->m_EndCol+1) - (r->m_StartCol-1)*r->m_StartCol) + ll *(c.x*c.y - c.x*r->m_Row);
    }

    c_xx/=A;c_yy/=A;c_xy/=A;
    res->m_Inertia = c_xx + c_yy;
    double T = sqrt( (c_xx - c_yy) * (c_xx - c_yy) + 4*c_xy*c_xy);
    res->m_Eccentricity = ((c_xx + c_yy) + T)/((c_xx+c_yy) - T);
    res->m_Orientation = 0.5*atan2(2*c_xy,c_xx-c_yy);
    //the l_i are variances (unit L^2) so to arrive at numbers that 
    //correspond to lengths in the picture we use sqrt
    if (fabs(c_xy) > 1e-30) {
        //FIXME. check l1!=0 l2!=0. li=0 happens for line-like components
        l1 = 0.5 * ( (c_xx+c_yy) + sqrt( (c_xx+c_yy)*(c_xx+c_yy) - 4 * (c_xx*c_yy-c_xy*c_xy) ) );
        l2 = 0.5 * ( (c_xx+c_yy) - sqrt( (c_xx+c_yy)*(c_xx+c_yy) - 4 * (c_xx*c_yy-c_xy*c_xy) ) );
        tmp_x = c_xy/l1 - c_xx*c_yy/(c_xy*l1)+ (c_xx/c_xy);
        tmp_y = 1.;
        mag = sqrt(tmp_x*tmp_x + tmp_y*tmp_y);
        res->m_EigenVectors[0].x = tmp_x/mag;
        res->m_EigenVectors[0].y = tmp_y/mag;
        res->m_EigenValues.x = l1;
        tmp_x = c_xy/l2 - c_xx*c_yy/(c_xy*l2)+ (c_xx/c_xy);
        tmp_y = 1.;
        mag = sqrt(tmp_x*tmp_x + tmp_y*tmp_y);
        res->m_EigenVectors[1].x = tmp_x/mag;
        res->m_EigenVectors[1].y = tmp_y/mag;
        res->m_EigenValues.y = l2;
    }else{
        //matrix already diagonal
        if (c_xx > c_yy) {
            res->m_EigenVectors[0].x = 1;
            res->m_EigenVectors[0].y = 0;
            res->m_EigenVectors[1].x = 0;
            res->m_EigenVectors[1].y = 1;
            res->m_EigenValues.x = c_xx;
            res->m_EigenValues.y = c_yy;
        } else {
            res->m_EigenVectors[0].x = 0;
            res->m_EigenVectors[0].y = 1;
            res->m_EigenVectors[1].x = 1;
            res->m_EigenVectors[1].y = 0;
            res->m_EigenValues.x = c_yy;
            res->m_EigenValues.y = c_xx;
        }
    }
    res->m_ScaledBasis[0].x = res->m_EigenVectors[0].x*sqrt(res->m_EigenValues.x);
    res->m_ScaledBasis[0].y = res->m_EigenVectors[0].y*sqrt(res->m_EigenValues.x);
    res->m_ScaledBasis[1].x = res->m_EigenVectors[1].x*sqrt(res->m_EigenValues.y);
    res->m_ScaledBasis[1].y = res->m_EigenVectors[1].y*sqrt(res->m_EigenValues.y);
    return res;
}
/*
double Blob::stddev(){
    DPoint c = center();

double res = 0;
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();r++){
        //This is the evaluated expression for the variance when using runs...
        res +=
                (*r)->length()*( ((*r)->m_Row - c.y) *((*r)->m_Row - c.y)) +
                ( (*r)->m_EndCol*((*r)->m_EndCol+1)*(2*(*r)->m_EndCol+1)-((*r)->m_StartCol-1)*((*r)->m_StartCol)*(2*(*r)->m_StartCol-1))/6. + 
                (*r)->length() * c.x*c.x - 
                c.x *((*r)->m_EndCol*((*r)->m_EndCol+1) - ((*r)->m_StartCol-1)* ((*r)->m_StartCol));

    }
    return sqrt(res/area());
}
*/

int connected(Run &r1, Run &r2)
{
    int res=0;
//    if (abs(r2.m_Row - r1.m_Row) != 1)
//        return 0;
    if (r1.m_StartCol > r2.m_StartCol){
        //use > here to do 8-connectivity
        res = r2.m_EndCol >= r1.m_StartCol;
    }else{
        res = r1.m_EndCol >= r2.m_StartCol;
    }
    return res;
}

void store_runs(CompsMap  *comps, RunList *runs1, RunList *runs2)
{
   BlobPtr p_blob;
   BlobPtr c_blob;
   for (RunList::iterator run1_it = runs1->begin(); run1_it!=runs1->end(); ++run1_it){
       for (RunList::iterator run2_it = runs2->begin(); run2_it!=runs2->end(); ++run2_it){
           if ( (run1_it->m_Color == run2_it->m_Color) && connected(*run1_it, *run2_it)){
               p_blob = comps->find(run1_it->m_Label)->second;
               c_blob = comps->find(run2_it->m_Label)->second;
                while (p_blob->m_pParent){
                    p_blob = p_blob->m_pParent;
                }
                while (c_blob->m_pParent){
                    c_blob = c_blob->m_pParent;
                }
                if (c_blob==p_blob){
                    //pass
                    ;
                }else{
                    p_blob->merge(c_blob); //destroys c_blobs runs_list
                    c_blob->m_pParent = p_blob;
                }
           }

       }
   }
}

Run new_run(CompsMap *comps, int row, int col1, int col2, int color)
{
    Run run = Run(row, col1, col2, color);
    BlobPtr b = BlobPtr(new Blob(run));
    (*comps)[run.m_Label] = b;
    return run;
}

void findRunsInLine(BitmapPtr pBmp, int y, CompsMap *comps, RunList * pRuns, 
        unsigned char threshold)
{
    int run_start=0;
    int run_stop=0;
    const unsigned char * pPixel = pBmp->getPixels()+y*pBmp->getStride();
    unsigned char cur=(*pPixel>threshold)?1:0;
    unsigned char p;
    int Width = pBmp->getSize().x;
    for(int x=0; x<Width; x++) {
        p = (*pPixel>threshold)?1:0;
        if (cur!=p) {
            if (cur) {
                if (x-run_start > 1) {
                    // Single light pixels are ignored.
                    run_stop = x - 1;
                    pRuns->push_back ( new_run(comps, y, run_start, run_stop, cur) );
                    run_start = x;
                }
            } else {
                run_stop = x - 1;
                if (run_stop-run_start == 0 && !pRuns->empty()) {
                    // Single dark pixels are ignored.
                    Run * pLastRun = &(pRuns->back());
                    run_start = pLastRun->m_StartCol;
                    comps->erase(pLastRun->m_Label);
                    pRuns->pop_back();
                } else {
                    run_start = x;
                }
            }
            cur = p;
        }
        pPixel++;
    }
    if (cur){
        pRuns->push_back( new_run(comps, y, run_start, Width, cur) );
    }

}

BlobListPtr connected_components(BitmapPtr image, unsigned char threshold)
{
    assert(image->getPixelFormat() == I8);
    CompsMap *comps = new CompsMap();
    IntPoint size = image->getSize();
    RunList *runs1=new RunList();
    RunList *runs2=new RunList();
    RunList *tmp;

    int y=0;
    findRunsInLine(image, 0, comps, runs1, threshold);
    
    for(y=1; y<size.y; y++){
        findRunsInLine(image, y, comps, runs2, threshold);
        store_runs(comps, runs1, runs2);
        tmp = runs1;
        runs1 = runs2;
        runs2 = tmp;
        runs2->clear();
    }
    BlobList *result = new BlobList();
    for (CompsMap::iterator b=comps->begin();b!=comps->end();++b){
        if (! b->second->m_pParent){
            result->push_back(b->second);
        }
    }
    //delete comps!
    comps->clear();
    delete comps;
    delete runs1;
    delete runs2;
    return BlobListPtr(result);
}

}
