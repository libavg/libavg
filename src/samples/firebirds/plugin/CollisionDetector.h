#ifndef _COLLISION_DETECTOR_H_
#define _COLLISION_DETECTOR_H_

#define AVG_PLUGIN

#include "../../../wrapper/WrapHelper.h"
#include "../../../graphics/Bitmap.h"


namespace avg {

class CollisionDetector
{
public:
    CollisionDetector(BitmapPtr bmpA, BitmapPtr bmpB);
    ~CollisionDetector();

    bool detect(glm::vec2 posA, glm::vec2 posB);

private:
    BitmapPtr m_pBmpA;
    BitmapPtr m_pBmpB;
};

}


#endif

