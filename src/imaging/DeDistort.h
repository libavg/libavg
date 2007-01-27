#ifndef _DeDistort_H_
#define _DeDistort_H_

#include "CoordTransformer.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <vector>
#include <boost/shared_ptr.hpp>

namespace avg {

struct DPoint3 {
public:
    DPoint3(double ix, double iy, double iz)
        : x(ix),
          y(iy),
          z(iz)
    {}

    double x;
    double y;
    double z;
};

class DeDistort: public CoordTransformer {
    public:
        DeDistort::DeDistort(const DPoint& FilmDisplacement, const DPoint& FilmScale, 
                const std::vector<double>& DistortionParams, 
                DPoint3& P, DPoint3& N, double Angle, 
                const DPoint& DisplayDisplacement, const DPoint& DisplayScale);
        virtual ~DeDistort();
        virtual DPoint transform_point(const DPoint & pt); //(x,y) -> (x', y')
        virtual DPoint inverse_transform_point(const DPoint & pt); //(x,y) -> (x', y')
        double getPixelSize(const DPoint & pt); //A(x,y) -> A'(x',y')
    private:
        DPoint undistort(const std::vector<double> &params, const DPoint &pt) ;
        DPoint scale(const DPoint &scales, const DPoint &pt);
        DPoint translate(const DPoint &displacement, const DPoint &pt);
        DPoint rotate(double angle, const DPoint &pt);
        DPoint pinhole(const DPoint3& P, const DPoint3& N, const DPoint &pt);

        DPoint m_FilmDisplacement;
        DPoint m_FilmScale;
        double m_Angle;
        DPoint3 m_P;
        DPoint3 m_N;
        std::vector<double> m_DistortionParams;
        DPoint m_DisplayDisplacement;
        DPoint m_DisplayScale;
};

}
#endif
