#ifndef _TouchEvent_h_
#define _TouchEvent_h_

#include "CursorEvent.h"
#include "Node.h"
#include "../graphics/Point.h"
#include "../graphics/Bitmap.h"
#include "../imaging/ConnectedComps.h"

#include "math.h"
namespace avg {
class TouchEvent: public CursorEvent {
    protected:
        BlobInfo m_Info;
        BlobPtr m_Blob;
    public:
        TouchEvent(int id, Type EventType, BlobInfo &info, BlobPtr blob);
        virtual Event* cloneAs(Type EventType);

        double getOrientation(){return m_Info.m_Orientation;};
        double getArea(){return m_Info.m_Area;};
        IntRect getBoundingBox(){return m_Info.m_BoundingBox;};
        double getInertia(){return m_Info.m_Inertia;};
        DPoint getCenter(){return m_Info.m_Center;};
        double getEccentricity(){return m_Info.m_Eccentricity;};
        DPoint getEigenValues(){return m_Info.m_EigenValues;};
        //BitmapPtr getBitmap();
        //DPoint[2] getScaledBasis(){return m_Info.m_ScaledBasis;};
};

}
#endif
