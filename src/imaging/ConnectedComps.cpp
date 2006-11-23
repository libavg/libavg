#include "ConnectedComps.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>


namespace avg {

int Run::last_label = 0;

Run::Run(int row, int start_col, int end_col, int color){
    m_Row = row;
    assert(end_col>=start_col);
    m_StartCol = start_col;
    m_EndCol = end_col;
    m_Color = color;
    last_label++;
    m_Label = last_label;
}

int Run::length(){
    return m_EndCol-m_StartCol+1;
}

DPoint Run::center(){
    DPoint d = DPoint(m_StartCol + length()/2., m_Row);
    return d;
}
Blob::Blob(RunPtr run) {
    m_pRuns = new RunList();
    m_pRuns->push_back(run);
    m_pParent = BlobPtr();
}

Blob::~Blob() {
    m_pRuns->clear();
    delete m_pRuns;
}

RunList *Blob::getlist(){
    return m_pRuns;
}

void Blob::merge(BlobPtr other) {
    m_pRuns->splice(m_pRuns->end(), *(other->getlist()));
}

DPoint Blob::center() {
    DPoint d = DPoint(0,0);
    int c = 0;
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();r++){
        d+=(*r)->center();
        c++;
    }

    return d/double(c);
}
IntRect Blob::bbox(){
    int x1=__INT_MAX__,y1=__INT_MAX__,x2=0,y2=0;
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();r++){
        x1 = std::min(x1, (*r)->m_StartCol);
        y1 = std::min(y1, (*r)->m_Row);
        x2 = std::max(x2, (*r)->m_EndCol);
        y2 = std::min(y2, (*r)->m_Row);
    }
    return IntRect(x1,y1,x2,y2);

}

int Blob::area(){
    int res = 0;
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();r++){
        res+= (*r)->length();
    }
    return res;
}

BitmapPtr Blob::render(){
    //Create a new Bitmap of bbox dimension
    return BitmapPtr();
}
BlobInfoPtr Blob::getInfo(){
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
    for(RunList::iterator r=m_pRuns->begin();r!=m_pRuns->end();r++){
        //This is the evaluated expression for the variance when using runs...
        ll = (*r)->length();
        c_yy += ll* ((*r)->m_Row- c.y)*((*r)->m_Row- c.y);
        c_xx += ( (*r)->m_EndCol * ((*r)->m_EndCol+1) * (2*(*r)->m_EndCol+1) - ((*r)->m_StartCol-1) * (*r)->m_StartCol * (2*(*r)->m_StartCol -1))/6. - 
            c.x * ( (*r)->m_EndCol*((*r)->m_EndCol+1) - ((*r)->m_StartCol-1)*(*r)->m_StartCol  )
            + ll* c.x*c.x;
        c_xy += ((*r)->m_Row-c.y)*0.5*( (*r)->m_EndCol*((*r)->m_EndCol+1) - ((*r)->m_StartCol-1)*(*r)->m_StartCol) + ll *(c.x*c.y - c.x*(*r)->m_Row);
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
    res->m_ScaledBasis[0].x = res->m_EigenVectors[0].x/sqrt(res->m_EigenValues.x);
    res->m_ScaledBasis[0].y = res->m_EigenVectors[0].y/sqrt(res->m_EigenValues.x);
    res->m_ScaledBasis[1].x = res->m_EigenVectors[1].x/sqrt(res->m_EigenValues.y);
    res->m_ScaledBasis[1].y = res->m_EigenVectors[1].y/sqrt(res->m_EigenValues.y);
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
int connected(RunPtr r1, RunPtr r2){
    int res=0;
    if (abs(r2->m_Row - r1->m_Row) != 1)
        return 0;
    if (r1->m_StartCol > r2->m_StartCol){
        //use > here to do 8-connectivity
        res = r2->m_EndCol >= r1->m_StartCol;
    }else{
        res = r1->m_EndCol >= r2->m_StartCol;
    }
    return res;
}
void store_runs(CompsMap  *comps, RunList *runs1, RunList *runs2){
   BlobPtr p_blob;
   BlobPtr c_blob;
   for (RunList::iterator run1_it = runs1->begin(); run1_it!=runs1->end(); run1_it++){
       for (RunList::iterator run2_it = runs2->begin(); run2_it!=runs2->end(); run2_it++){
           if ( ((*run1_it)->m_Color == (*run2_it)->m_Color) && connected(*run1_it, *run2_it)){
               p_blob = comps->find((*run1_it)->m_Label)->second;
               c_blob = comps->find((*run2_it)->m_Label)->second;
                while (p_blob->m_pParent)
                    p_blob = p_blob->m_pParent;
                while (c_blob->m_pParent)
                    c_blob = c_blob->m_pParent;
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


RunPtr new_run(CompsMap *comps, int row, int col1, int col2, int color)
{
    RunPtr run = RunPtr(new Run(row, col1, col2, color));
    BlobPtr b = BlobPtr(new Blob(run));
    //std::cerr<<"creating new run"<<"row="<<row<<" c1="<<col1<<" c2="<<col2<<" color="<<color<<std::endl;;
    (*comps)[run->m_Label] = b;
    return run;
}

BlobListPtr connected_components(BitmapPtr image, int object_threshold){
    assert(image->getPixelFormat() == I8);
    CompsMap *comps = new CompsMap();
    const unsigned char *pixels = image->getPixels();
    int stride = image->getStride();
    IntPoint size = image->getSize();
    RunList *runs1=new RunList();
    RunList *runs2=new RunList();
    RunList *tmp;

    int run_start=0, run_stop=0;
    unsigned char cur=(pixels[0]>object_threshold)?1:0, p=0;
    std::cerr<<"w="<<size.x<<" h="<<size.y<<std::endl;
    //First line
    for(int x=0; x<size.x ;x++){
        p = (pixels[x]>object_threshold)?1:0;
        if (cur!=p) {
            run_stop = x - 1;
            if (cur)
                runs1->push_back ( new_run(comps, 0, run_start, run_stop, cur) );
            run_start = x;
            cur = p;
        }
    }
    if (cur)
        runs1->push_back( new_run(comps, 0, run_start, size.x-1, cur) );
    //All other lines
    for(int y=1; y<size.y; y++){
        run_start = 0;run_stop = 0;
        cur = (pixels[stride*y+0]>object_threshold)?1:0;
        for(int x=0; x<size.x ;x++){
            p = (pixels[y*stride+x]>object_threshold)?1:0;
            //std::cerr<<"("<<x<<","<<y<<"):"<<(int)p<<std::endl;
            if (cur!=p) {
                run_stop = x - 1;
                if (cur)
                    runs2->push_back(  new_run(comps, y, run_start, run_stop, cur) );
                run_start = x;
                cur = p;
            }
        }
        {
            if (cur)
                runs2->push_back( new_run(comps,y, run_start, size.x-1, cur) );
            store_runs(comps, runs1, runs2);
            tmp = runs1;
            runs1 = runs2;
            runs2 = tmp;
            runs2->clear();
        }
    }
    BlobList *result = new BlobList();
    for (CompsMap::iterator b=comps->begin();b!=comps->end();b++){
        if (! b->second->m_pParent){
            result->push_back(b->second);
        }
    }
    //delete comps!
    comps->clear();
    delete comps;
        
    return BlobListPtr(result);
}
}
