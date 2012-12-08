#ifndef _COLLISION_DETECTOR_H_
#define _COLLISION_DETECTOR_H_

#define AVG_PLUGIN

#include <graphics/Bitmap.h>


namespace avg {

class CollisionDetector
{
public:
    CollisionDetector(const Bitmap& bmpA, const Bitmap& bmpB);
    ~CollisionDetector();

    bool detect(glm::vec2 posA, glm::vec2 posB);

private:
    Bitmap* m_pBmpA;
    Bitmap* m_pBmpB;
};

}


#endif

