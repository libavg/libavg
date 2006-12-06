#ifndef _TrackerCmd
#define _TrackerCmd
#include <boost/shared_ptr.hpp>

namespace avg {

struct TrackerConfig{
    TrackerConfig(int Brightness, int Exposure, int Gain, int Shutter,
            int Threshold, double Similarity, double MinArea, double MaxArea, 
            double MinEccentricity, double MaxEccentricity);
    int m_Brightness;
    int m_Exposure;
    int m_Gain;
    int m_Shutter;
    int m_Threshold; //min pixel val for detection
    double m_Similarity; //max distance for tracking blobs
    double m_AreaBounds[2]; //min, max for area in percents of screen size
    double m_EccentricityBounds[2]; //min, max for Eccentricity
};

}
#endif
