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
        return trapezoid(distortion(pt));
    }
    double getPixelSize(const DPoint &pt){
        return 1; // FIXME
    }
}
