#ifndef _TrapezoidAndBarrel_H_
#define _TrapezoidAndBarrel_H_

#include "CoordTransformer.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <boost/shared_ptr.hpp>

namespace avg {
class TrapezoidAndBarrel: public CoordTransformer
{
    public:
        TrapezoidAndBarrel(IntRect srcRect, double K1, double T, double RescaleFactor = 1);
        virtual ~TrapezoidAndBarrel();
        virtual DPoint transform_point(const DPoint & pt); //(x,y) -> (x', y')
        virtual DPoint inverse_transform_point(const DPoint & pt); //(x,y) -> (x', y')
        //l_x = srcRect.width
        //l_y = srcRect.height
        //c_x = l_x/2.
        //c_y = l_y/2.
        //
        //D = sqrt( (c_x-l_x)**2 + (c_y-l_y)**2)
        //def f(x, y):
        //    xn = (x-c_x)/D
        //    yn = (y-c_y)/D
        //    r_d = sqrt( xn**2 + yn**2 )
        //    S = (1 + k1 * r_d**2)
        //    return xn*S*D+c_x, yn*S*D+c_y
        //
        double getPixelSize(const DPoint & pt); //A(x,y) -> A'(x',y')
    private:
        DPoint trapezoid(const DPoint &pt);
        DPoint distortion(const DPoint &pt);
        DPoint inv_trapezoid(const DPoint &pt);
        DPoint inv_distortion(const DPoint &pt);
        double m_TrapezoidFactor;
        //pincushion/barrel correction
        //K1<0 correct barrel
        //K1>0 correct pincushion
        //see http://www.imatest.com/docs/distortion.html
        DPoint m_Center;
        double m_K1;
        double m_RescaleFactor;
        double m_Scale;
        double m_TrapezoidScale;
        //DPoint **m_pCache; // m_pCache[x][y] - (new_x, new_y)
};

}
#endif
