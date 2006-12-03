//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de

#include "TrackerCmd.h"
#include "TrackerThread.h"

namespace avg {
    TrackerConfig::TrackerConfig(int Brightness, int Exposure,
            int Threshold, double Similarity, 
            double MinArea, double MaxArea, 
            double MinEccentricity, double MaxEccentricity)
        : m_Brightness(Brightness),
          m_Exposure(Exposure),
          m_Threshold(Threshold),
          m_Similarity(Similarity)
    {
          m_AreaBounds[0] = MinArea;
          m_AreaBounds[1] = MaxArea;
          m_EccentricityBounds[0] = MinEccentricity; 
          m_EccentricityBounds[1] = MaxEccentricity;
    } 

    TrackerStopCmd::TrackerStopCmd()
    {
    }

    void TrackerStopCmd::execute(TrackerThread* pTarget)
    {
        pTarget->stop();
    }

    TrackerThresholdCmd::TrackerThresholdCmd(int Threshold)
      : m_Threshold(Threshold)
    {
    }

    void TrackerThresholdCmd::execute(TrackerThread* pTarget)
    {
        pTarget->setThreshold(m_Threshold);
    }

    TrackerBrightnessCmd::TrackerBrightnessCmd(int Brightness)
      : m_Brightness(Brightness)
    {
    }

    void TrackerBrightnessCmd::execute(TrackerThread* pTarget)
    {
        pTarget->setBrightness(m_Brightness);
    }

    TrackerExposureCmd::TrackerExposureCmd(int Exposure)
      : m_Exposure(Exposure)
    {
    }

    void TrackerExposureCmd::execute(TrackerThread* pTarget)
    {
        pTarget->setExposure(m_Exposure);
    }
}
