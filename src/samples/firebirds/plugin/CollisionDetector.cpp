#include "CollisionDetector.h"

#include "../../../base/ScopeTimer.h"
#include "../../../wrapper/WrapHelper.h"

namespace avg {

CollisionDetector::CollisionDetector(const Bitmap& bmpA, const Bitmap& bmpB)
{
    m_pBmpA = new Bitmap(bmpA.getSize(), B8G8R8A8);
    m_pBmpA->copyPixels(bmpA);
    m_pBmpB = new Bitmap(bmpB.getSize(), B8G8R8A8);
    m_pBmpB->copyPixels(bmpB);
}

CollisionDetector::~CollisionDetector()
{
    delete m_pBmpA;
    delete m_pBmpB;
}

static ProfilingZoneID CollisionDetectorProfilingZone("Detect collisions");

bool CollisionDetector::detect(glm::vec2 posA, glm::vec2 posB)
{
    ScopeTimer Timer(CollisionDetectorProfilingZone);

    int widthA = m_pBmpA->getSize().x;
    int heightA = m_pBmpA->getSize().y;
    int rightA = posA.x + widthA;
    int bottomA = posA.y + heightA;
    int widthB = m_pBmpB->getSize().x;
    int heightB = m_pBmpB->getSize().y;
    int rightB = posB.x + widthB;
    int bottomB = posB.y + heightB;

    // bounding box test
    if (rightA <= posB.x || posA.x >= rightB || bottomA <= posB.y || posA.y >= bottomB) {
        return false;
    }

    // pixel level test

    // calculate x overlap
    int dX = posB.x - posA.x;
    if (dX < 0) {
        posA.x = 0;
        posB.x = -dX;
    }
    else {
        posA.x = dX;
        posB.x = 0;
    }
    int width;
    dX = rightB - rightA;
    if (dX < 0)
        width = std::min(widthA + dX, widthB);
    else
        width = std::min(widthB - dX, widthA);
    // calculate y overlap
    int dY = posB.y - posA.y;
    if (dY < 0) {
        posA.y = 0;
        posB.y = -dY;
    }
    else {
        posA.y = dY;
        posB.y = 0;
    }
    int height;
    dY = bottomB - bottomA;
    if (dY < 0)
        height = std::min(heightA + dY, heightB);
    else
        height = std::min(heightB - dY, heightA);

    // test alpha channels
    const unsigned char* pPixStartA = m_pBmpA->getPixels() +
            (int) posA.y * m_pBmpA->getStride() +
            (int) posA.x * 4 + 3; // alpha channel of 1st A pixel
    const unsigned char* pPixStartB = m_pBmpB->getPixels() +
            (int) posB.y * m_pBmpB->getStride() +
            (int) posB.x * 4 + 3; // alpha channel of 1st B pixel
    const unsigned char* pPixA = pPixStartA;
    const unsigned char* pPixB = pPixStartB;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (*pPixA && *pPixB) {
                return true; // both pixels have alpha > 0 --> collision
            }
            pPixA += 4;
            pPixB += 4;
        }
        pPixA = pPixStartA + y * m_pBmpA->getStride();
        pPixB = pPixStartB + y * m_pBmpB->getStride();
    }

    return false;
}

} // namespace avg


using namespace boost::python;

BOOST_PYTHON_MODULE(collisiondetector)
{
    class_<avg::CollisionDetector, boost::noncopyable>("CollisionDetector", no_init)
        .def(init<avg::Bitmap&, avg::Bitmap&>())
        .def("detect", &avg::CollisionDetector::detect)
        ;
}


AVG_PLUGIN_API PyObject* registerPlugin()
{
#if PY_MAJOR_VERSION < 3
    initcollisiondetector(); // created by BOOST_PYTHON_MODULE
    return PyImport_ImportModule("collisiondetector");
#else
    throw Exception(AVG_ERR_UNSUPPORTED, "Python3 not supported yet")
#endif
}

