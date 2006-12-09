#ifndef _TrackerConfig
#define _TrackerConfig

#include <string>

namespace avg {

struct TrackerConfig{
    TrackerConfig();
    virtual ~TrackerConfig();
    
    void load(std::string sFilename);
    void save(std::string sFilename);

    int m_Brightness;
    int m_Exposure;
    int m_Gain;
    int m_Shutter;
    int m_Threshold; //min pixel val for detection
    int m_HistoryUpdateInterval;
    double m_Similarity; //max distance for tracking blobs
    double m_AreaBounds[2]; //min, max for area in percents of screen size
    double m_EccentricityBounds[2]; //min, max for Eccentricity
};

}
#endif
