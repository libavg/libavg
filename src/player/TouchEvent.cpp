#include "TouchEvent.h"
#include "../graphics/Bitmap.h"
#include "../imaging/ConnectedComps.h"

#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

namespace avg {
TouchEvent::TouchEvent(int id, Type EventType, BlobInfo &info, BlobPtr blob):
    CursorEvent(id, EventType, int(round(info.m_Center.x)), int(round(info.m_Center.y))),
    m_Blob(blob),
    m_Info(info){}
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
}

