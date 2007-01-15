#ifndef _TrackerConfig
#define _TrackerConfig

#include <string>
#include "../graphics/Rect.h"
namespace avg {

struct TrackerConfig{
    TrackerConfig();
    virtual ~TrackerConfig();
    
    void load(std::string sFilename);
    void save(std::string sFilename);

    // Coordinate transforms.
    IntRect m_ROI;//after applying de-distortion, take this as the table surface
    double m_K1; //amount of barrel/pincushion distortion to correct
    double m_T; //amount of trapezoid distortion to correct
    IntRect m_DestRect; // Transform all blobs to be in this rect at the end
    
    // Camera params
    int m_Brightness;
    int m_Exposure;
    int m_Gamma;
    int m_Gain;
    int m_Shutter;

    // Imaging params
    int m_Threshold; //min pixel val for detection
    int m_HistoryUpdateInterval;
    double m_Similarity; //max distance for tracking blobs
    double m_AreaBounds[2]; //min, max for area in percents of screen size
    double m_EccentricityBounds[2]; //min, max for Eccentricity

    bool m_bDebug;
};

}
#endif
