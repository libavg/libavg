#include "TouchEvent.h"

#include "../imaging/ConnectedComps.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

#include "../base/Logger.h"


namespace avg {
TouchEvent::TouchEvent(int id, Type EventType, BlobInfoPtr info, BlobPtr blob, 
        DPoint& Offset, DPoint& Scale)
    : CursorEvent(id, EventType, transformPoint(info->m_Center, Offset, Scale)),
      m_Info(info),
      m_Blob(blob)
{
}

Event* TouchEvent::cloneAs(Type EventType){
    TouchEvent *res = new TouchEvent(*this);
    res->m_Type = EventType;
    return res;
}
#ifdef BROKEN
BitmapPtr TouchEvent::getBitmap() {
    IntRect bb = getBoundingBox();
    IntPoint img_size = IntPoint(bb.Width(),bb.Height());
    BitmapPtr res = BitmapPtr(new Bitmap(img_size, I8));
    FilterFill<Pixel8>(0).applyInPlace(res);
    m_Blob->render(res, m_Blob);
    return res;
}
#endif

void TouchEvent::trace()
{
    Event::trace();
    AVG_TRACE(Logger::EVENTS2, "pos: " << m_Position 
            << ", ID: " << getCursorID()
            << ", Area: " << m_Info->m_Area
            << ", Eccentricity: " << m_Info->m_Eccentricity);
}
      
IntPoint TouchEvent::transformPoint(DPoint& pt, DPoint& Offset, DPoint& Scale)
{
    return IntPoint(
            int(round(pt.x*Scale.x)+Offset.x), 
            int(round(pt.y*Scale.y)+Offset.y));
}

}

