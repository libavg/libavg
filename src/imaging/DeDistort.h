#ifndef _DeDistort_H_
#define _DeDistort_H_

#include "CoordTransformer.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <vector>
#include <boost/shared_ptr.hpp>

namespace avg {
class DeDistort: public CoordTransformer
{
    public:
        DeDistort::DeDistort(DPoint &FilmDisplacement, 
                DPoint &FilmScale, 
                std::vector<double> DistortionParams, 
                double P[3], 
                double N[3], 
                double Angle, 
                DPoint m_DisplayScale, 
                DPoint m_DisplayDisplacement );
        virtual ~DeDistort();
        virtual DPoint transform_point(const DPoint & pt); //(x,y) -> (x', y')
        virtual DPoint inverse_transform_point(const DPoint & pt); //(x,y) -> (x', y')
        double getPixelSize(const DPoint & pt); //A(x,y) -> A'(x',y')
    private:
        DPoint undistort(std::vector<double> &params, DPoint &pt) ;
        DPoint scale(DPoint &scales, DPoint &pt);
        DPoint translate(DPoint &displacement, DPoint &pt);
        DPoint rotate(double angle, DPoint &pt);
        DPoint pinhole(double normal_vec_1, double normal_vec_2, double normal_vec_3, 
        double pinhole_position_1, double pinhole_position_2, double pinhole_position_3,
        DPoint &pt) ;


        DPoint m_DisplayDisplacement;
        DPoint m_DisplayScale;
        double m_Angle;
        double m_P[3];
        double m_N[3];
        std::vector<double> m_DistortionParams;
        DPoint m_FilmScale;
        DPoint m_FilmDisplacement;
};

}
#endif
