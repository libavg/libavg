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
    ContribDef(double dWidth) 
        : m_dWidth (dWidth)
    {}

    virtual ~ContribDef() {}

    double GetWidth() const   
    { 
        return m_dWidth; 
    }

    void SetWidth(double dWidth)
    { 
        m_dWidth = dWidth; 
    }

    virtual double Filter(double dVal) const = 0;

protected:
    double  m_dWidth;
};

class BilinearContribDef : public ContribDef
{
public:
    BilinearContribDef(double dWidth = 1.0) 
        : ContribDef(dWidth)
    {}

    virtual ~BilinearContribDef() {}

    virtual double Filter(double dVal) const
    {
        dVal = fabs(dVal);
        return (dVal < m_dWidth ? m_dWidth - dVal : 0.0);
    }
};

class GaussianContribDef : public ContribDef
{
public:
    GaussianContribDef(double dWidth = 3.0)
        : ContribDef(dWidth) 
    {}

    virtual ~GaussianContribDef() {}

    virtual double Filter(double dVal) const
    {
        if (fabs (dVal) > m_dWidth)
        {
            return 0.0;
        }
        return exp (-dVal * dVal / m_dWidth-1) / sqrt (2*PI);
    }
};

}

#endif
