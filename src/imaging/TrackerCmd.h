#ifndef _TrackerCmd
#define _TrackerCmd
#include <boost/shared_ptr.hpp>

namespace avg {

class TrackerThread;

struct TrackerConfig{
    TrackerConfig(int Brightness, int Exposure, 
            int Threshold, double Similarity, double MinArea, double MaxArea, 
            double MinEccentricity, double MaxEccentricity);
    int m_Brightness;
    int m_Exposure;
    int m_Threshold; //min pixel val for detection
    double m_Similarity; //max distance for tracking blobs
    double m_AreaBounds[2]; //min, max for area in percents of screen size
    double m_EccentricityBounds[2]; //min, max for Eccentricity
};

typedef boost::shared_ptr<TrackerConfig> TrackerConfigPtr;
class TrackerCmd {
public:
    virtual ~TrackerCmd() {};
    virtual void execute(TrackerThread* pTarget) = 0;
};
typedef boost::shared_ptr<TrackerCmd> TrackerCmdPtr;

class TrackerStopCmd: public TrackerCmd
{
public:
    TrackerStopCmd();
    void execute(TrackerThread* pTarget);
};

class TrackerThresholdCmd: public TrackerCmd
{
public:
    TrackerThresholdCmd(int Threshold);
    void execute(TrackerThread* pTarget);

private:
    int m_Threshold;
};

class TrackerBrightnessCmd: public TrackerCmd
{
public:
    TrackerBrightnessCmd(int Brightness);
    void execute(TrackerThread* pTarget);

private:
    int m_Brightness;
};

class TrackerExposureCmd: public TrackerCmd
{
public:
    TrackerExposureCmd(int Exposure);
    void execute(TrackerThread* pTarget);

private:
    int m_Exposure;
};

}
#endif
