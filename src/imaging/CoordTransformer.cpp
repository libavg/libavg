#include "CoordTransformer.h"
#include "../graphics/Rect.h"
#include "../graphics/Point.h"

#include "math.h"

namespace avg{
CoordTransformer::CoordTransformer(IntRect srcRect, double K1, double T)
    :m_K1(K1),
    m_TrapezoidFactor(T)
{
    m_Center = DPoint(srcRect.tl.x+srcRect.Width()/2, srcRect.tl.y+srcRect.Height()/3);
    //normalize to center-edge distance
    m_Scale = sqrt( pow(m_Center.x - srcRect.Width(),2) +  pow(m_Center.y - srcRect.Height(),2) );


}
    CoordTransformer::~CoordTransformer(){};
    DPoint CoordTransformer::distortion(const DPoint &pt){
        DPoint pt_norm = (pt - m_Center)/m_Scale;
        double r_d_squared = pt_norm.x*pt_norm.x + pt_norm.y*pt_norm.y;
        double S = (1 + m_K1*r_d_squared);
        return pt_norm*S*m_Scale + m_Center;
    }
    DPoint CoordTransformer::trapezoid(const DPoint &pt){
        //stretch x coord
        double yn = (pt.y - m_Center.y)/m_Scale;
        return DPoint( 
                m_Center.x + ( pt.x - m_Center.x) * (1 + m_TrapezoidFactor * yn), 
                pt.y);
    }
    DPoint CoordTransformer::transform_point(const DPoint &pt){
        return distortion(trapezoid(pt));
    }
    double CoordTransformer::getPixelSize(const DPoint &pt){
        //volume dxdy transforms as |D|dx'dy' where |D| is the functional determinant
        //det { { dx'/dx, dx'/dy}, {dy'/dx,dy'/dy}}
        //|D| = dx'/dx * dy'/dy - dx'/dy * dy'/dx
        //
        //with x'=x'(x,y) are the new coordinates in terms of the old ones
        //in our case.
        //trapezoid:
        //x' = m0 + (x-m0)*(1+m_TrapezoidFactor*yn)
        //y' = y
        //|D| = (1+m_TrapezoidFactor*yn)|(x,y)
        //
        //distortion:
        //x' = S*(x-x0) + x0
        //y' = S*(y-y0) + y0
        //oBdA x0=y0=0 (volume invariant under translation)
        //S = (1+m_K1*|(x,y)|^2/m_Scale)
        //dx'/dx = S+x*dS/dx
        //dx'/dy = x*dS/dy
        //dy'/dx = y*dS/dx
        //dy'/dy = S+y*dS/dx
        //maxima:
        //S(x,y):= (1+K1 *(x**2+y**2));
        //dS/dx = 2xK1
        //dS/dy = 2yK1
        //xd(x,y):=S(x,y)*x;
        //yd(x,y):=S(x,y)*y;
        //
        //diff(xd(x,y),x)*diff(yd(x,y),y)-diff(xd(x,y),y)*diff(yd(x,y),x);
        //|D| = S^2 + 2*x*y*dS/dx*dS/dy = S^2 + 16 x^2*y^2 K1^2
        //|D| = (...)|(xt,yt) where xt,yt are the coords after trapezoid trafo 
        double yn = (pt.y - m_Center.y)/m_Scale;
        DPoint pt2 = trapezoid(pt);
        return (1+m_TrapezoidFactor*yn); // FIXME
    }
    
}
