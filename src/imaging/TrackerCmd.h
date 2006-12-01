#ifndef _TrackerCmd
#define _TrackerCmd
#include <boost/shared_ptr.hpp>

namespace avg {
struct TrackerConfig{
        TrackerConfig(int Threshold, double Similarity, double MinArea, double MaxArea, 
                double MinEccentricity, double MaxEccentricity);
        int m_Threshold; //min pixel val for detection
        double m_Similarity; //max distance for tracking blobs
        double m_AreaBounds[2]; //min, max for area in percents of screen size
        double m_EccentricityBounds[2]; //min, max for Eccentricity

};

typedef boost::shared_ptr<TrackerConfig> TrackerConfigPtr;
struct TrackerCmd {
    typedef enum {CONFIG, RESET, STOP} CmdType;
    TrackerCmd(CmdType Cmd);
    TrackerConfigPtr config;//stupid static typing...

    CmdType m_Cmd;
};
typedef boost::shared_ptr<TrackerCmd> TrackerCmdPtr;
}
#endif
