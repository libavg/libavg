//
// $Id$
// 

#include "AVGJSPoint.h"

#include <xpcom/nsMemory.h>

NS_IMPL_ISUPPORTS1_CI(AVGJSPoint, IAVGJSPoint);

AVGJSPoint::AVGJSPoint ()
{
    NS_INIT_ISUPPORTS();
}

AVGJSPoint::~AVGJSPoint()
{
}

NS_IMETHODIMP AVGJSPoint::GetX(float *aX)
{
    *aX = x;
    return NS_OK;
}

NS_IMETHODIMP AVGJSPoint::SetX(float aX)
{
    x = aX;
    return NS_OK;
}

NS_IMETHODIMP AVGJSPoint::GetY(float *aY)
{
    *aY = y;
    return NS_OK;
}

NS_IMETHODIMP AVGJSPoint::SetY(float aY)
{
    y = aY;
    return NS_OK;
}

AVGJSPoint & AVGJSPoint::operator =(const AVGJSPoint & Pt)
{
    AVGDPoint::operator=(Pt);
    return *this;
}

AVGJSPoint & AVGJSPoint::operator =(const AVGDPoint & Pt)
{
    AVGDPoint::operator=(Pt);
    return *this;
}

