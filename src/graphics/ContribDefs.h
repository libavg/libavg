// Fast and accurate bitmap scaling. Original code by Eran Yariv and Jake Montgomery,
// posted on codeguru.com.

#ifndef _ContribDefs_h_
#define _ContribDefs_h_

#include "../base/MathHelper.h"

#include <math.h>

namespace avg {

class ContribDef
{
public:
    ContribDef(float dWidth) 
        : m_dWidth (dWidth)
    {}

    virtual ~ContribDef() {}

    float GetWidth() const   
    { 
        return m_dWidth; 
    }

    void SetWidth(float dWidth)
    { 
        m_dWidth = dWidth; 
    }

    virtual float Filter(float dVal) const = 0;

protected:
    float  m_dWidth;
};

class BilinearContribDef : public ContribDef
{
public:
    BilinearContribDef(float dWidth = 1.0) 
        : ContribDef(dWidth)
    {}

    virtual ~BilinearContribDef() {}

    virtual float Filter(float dVal) const
    {
        dVal = fabs(dVal);
        return (dVal < m_dWidth ? m_dWidth - dVal : 0.0f);
    }
};

class GaussianContribDef : public ContribDef
{
public:
    GaussianContribDef(float dWidth = 3.0)
        : ContribDef(dWidth) 
    {}

    virtual ~GaussianContribDef() {}

    virtual float Filter(float dVal) const
    {
        if (fabs (dVal) > m_dWidth)
        {
            return 0.0;
        }
        return float(exp(-dVal * dVal/m_dWidth-1) / sqrt(2*PI));
    }
};

}

#endif
