//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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
//

#include "WrapHelper.h"

#include "../player/BoostPython.h"
#include "../player/BitmapManager.h"

#include "../graphics/Bitmap.h"
#include "../graphics/BitmapLoader.h"
#include "../graphics/FilterResizeBilinear.h"
#include "../graphics/ImageCache.h"
#include "../graphics/Color.h"

#include "../base/CubicSpline.h"
#include "../base/GeomHelper.h"

#include "../glm/gtx/vector_angle.hpp"

#include <vector>
#include <sstream>

using namespace boost::python;
using namespace std;
using namespace avg;

namespace bp = boost::python;

template<class POINT>
class_<POINT> export_point(const string& sName)
{
    return class_<POINT>(sName.c_str(), no_init)
        .def("__len__", &Vec2Helper::len)
        .def("__getitem__", &Vec2Helper::getItem)
        .def("__str__", &Vec2Helper::str)
        .def("__repr__", &Vec2Helper::repr)
        .def("__hash__", &Vec2Helper::getHash)
        .def("getNormalized", &Vec2Helper::safeGetNormalized)
        .def("getNorm", &Vec2Helper::getNorm)
        .def("getRotated", &getRotated)
        .def("getRotated", &getRotatedPivot)
        .def(self == self)
        .def(self != self)
        .def(-self)
        .def(self + self)
        .def(self - self)
        .def(float() * self)
        .def(self * float())
        .def(self / float())
        .def("isInPolygon", &pointInPolygon)
        .def("getAngle", &getAngle)
        .def("fromPolar", &fromPolar)
        .staticmethod("fromPolar")
        .def("angle", &Vec2Helper::vecAngle)
        .staticmethod("angle")
    ;
}

struct Pixel32_to_python_tuple
{
    static PyObject* convert (avg::Pixel32 px)
    {
        return bp::incref(bp::make_tuple(
                px.getR(), px.getG(), px.getB(), px.getA()).ptr());
    }
};

static bp::object Bitmap_getPixels(Bitmap& bitmap, bool bCopyData=true)
{
    unsigned char* pBuffer = bitmap.getPixels();
    int buffSize = bitmap.getMemNeeded();
    if (bCopyData) {
        bp::object pyBuffer(handle<>(PyBuffer_New(buffSize)));
        void* pTargetBuffer;
        Py_ssize_t pyBuffSize  = buffSize;
        PyObject_AsWriteBuffer(pyBuffer.ptr(), &pTargetBuffer, &pyBuffSize);
        memcpy(pTargetBuffer, pBuffer, buffSize);
        return pyBuffer;
    } else {
#if PY_MAJOR_VERSION < 3
        return bp::object(handle<>(PyBuffer_FromMemory(pBuffer, buffSize)));
#else
        // TODO: This returns a memoryview and not a buffer, which will probably break
        // PIL interoperability.
        PyObject* py_memView = PyMemoryView_FromMemory((char*)pBuffer,
                buffSize, PyBUF_READ);
        return bp::object(handle<>(py_memView));
#endif
    }
}

BOOST_PYTHON_FUNCTION_OVERLOADS(Bitmap_getPixels_overloads, Bitmap_getPixels,
        1, 2);

static void Bitmap_setPixels(Bitmap& bitmap, PyObject* exporter, int stride=0)
{
    Py_buffer bufferView;
//    PyObject_Print(PyObject_Type(exporter), stderr, 0);
    if (PyObject_CheckBuffer(exporter)) {
        PyObject_GetBuffer(exporter, &bufferView, PyBUF_READ);
        const unsigned char* pBuf = reinterpret_cast<unsigned char*>(bufferView.buf);
        bitmap.setPixels(pBuf, stride);
        PyBuffer_Release(&bufferView);
    } else if (PyBuffer_Check(exporter)) {
        PyTypeObject * pType = exporter->ob_type;
        PyBufferProcs * pProcs = pType->tp_as_buffer;
        AVG_ASSERT(pProcs);
        Py_ssize_t numBytes;
        Py_ssize_t numSegments = pProcs->bf_getsegcount(exporter, &numBytes);
        if (numBytes != bitmap.getMemNeeded()) {
            throw Exception(AVG_ERR_INVALID_ARGS,
                    "Second parameter to Bitmap.setPixels must fit bitmap size.");
        }
        // TODO: check if bitmap size is correct
        unsigned char* pDestPixels = bitmap.getPixels();
        for (int i=0; i<numSegments; ++i) {
            void* pSrcPixels;
            long bytesInSegment = pProcs->bf_getreadbuffer(exporter, i, &pSrcPixels);
            memcpy(pDestPixels, pSrcPixels, bytesInSegment);
            pDestPixels += bytesInSegment;
        }
    } else {
        throw Exception(AVG_ERR_INVALID_ARGS,
               "Second parameter to Bitmap.setPixels must support the buffer interface.");
    }
}

BOOST_PYTHON_FUNCTION_OVERLOADS(Bitmap_setPixels_overloads, Bitmap_setPixels,
        2, 3);

ConstVec2 Bitmap_getSize(Bitmap* This)
{
    return (glm::vec2)(This->getSize());
}

BitmapPtr Bitmap_getResized(BitmapPtr This, const glm::vec2& size)
{
    return FilterResizeBilinear(IntPoint(size)).apply(This);
}

glm::vec2* createPoint()
{
    return new glm::vec2(0,0);
}

BitmapPtr createBitmapFromFile(const UTF8String& sFName)
{
    return loadBitmap(sFName);
}

BitmapPtr createBitmapWithRect(BitmapPtr pBmp,
        const glm::vec2& tlPos, const glm::vec2& brPos)
{
    if (tlPos.x >= brPos.x || tlPos.y >= brPos.y) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, 
                "Can't create a bitmap with zero or negative width/height.");
    }
    IntPoint size = pBmp->getSize();
    if (tlPos.x < 0 || tlPos.y < 0 || brPos.x > size.x || brPos.y > size.y) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, 
                "Attempt to create a subbitmap that doesn't fit into the parent bitmap.");
    }
    IntRect rect = IntRect(IntPoint(tlPos), IntPoint(brPos));
    return BitmapPtr(new Bitmap(*pBmp, rect));
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(loadBitmap_overloads, BitmapManager::loadBitmapPy, 
        2, 3);

static bp::object ImageCache_GetCapacity(ImageCache* pCache)
{
    return bp::make_tuple(pCache->getCapacity(CachedImage::STORAGE_CPU),
            pCache->getCapacity(CachedImage::STORAGE_GPU));
}

static void ImageCache_SetCapacity(ImageCache* pCache, const bp::tuple& cap)
{
    long long cpuCap = extract<long long>(cap[0]);
    long long gpuCap = extract<long long>(cap[1]);
    pCache->setCapacity(cpuCap, gpuCap);
}

static bp::object ImageCache_GetNumImages(ImageCache* pCache)
{
    return bp::make_tuple(pCache->getNumCPUImages(), pCache->getNumGPUImages());
}

static bp::object ImageCache_GetMemUsed(ImageCache* pCache)
{
    return bp::make_tuple(pCache->getMemUsed(CachedImage::STORAGE_CPU),
            pCache->getMemUsed(CachedImage::STORAGE_GPU));
}

vector<string> getSupportedPixelFormatsDeprecated()
{
    avgDeprecationWarning("1.9.0", "avg.getSupportedPixelFormats",
            "avg.Bitmap.getSupportedPixelFormats");
    return getSupportedPixelFormats();
}


void export_bitmap()
{
    export_point<glm::vec2>("Point2D")
        .def("__init__", make_constructor(createPoint))
        .def(init<float, float>())
        .def(init<const glm::vec2&>())
        .def("__setitem__", &Vec2Helper::setItem)
        .add_property("x", &Vec2Helper::getX, &Vec2Helper::setX, "")
        .add_property("y", &Vec2Helper::getY, &Vec2Helper::setY, "")
    ;

    export_point<ConstVec2>("ConstPoint2D")
        .add_property("x", &Vec2Helper::getX, "")
        .add_property("y", &Vec2Helper::getY, "")
    ;

    implicitly_convertible<ConstVec2, glm::vec2>();
    implicitly_convertible<glm::vec2, ConstVec2>();

    class_<Color, boost::shared_ptr<Color> >("Color", no_init)
        .def(init<string>())
        .def(init<unsigned char, unsigned char, unsigned char>())
        .def(init<const glm::vec3&>())
        .add_property("r", &Color::getR, "")
        .add_property("g", &Color::getG, "")
        .add_property("b", &Color::getB, "")
        .def(self == self)
        .def(self != self)
        .def(self_ns::str(self))
        .def("mix", &Color::mix)
        .staticmethod("mix")
        .def("fromLch", &Color::fromLch)
        .staticmethod("fromLch")
    ;
    implicitly_convertible<glm::vec3, Color>();
    implicitly_convertible<string, Color>();

    enum_<PixelFormat>("pixelformat")
        .value("B5G6R5", B5G6R5)
        .value("B8G8R8", B8G8R8)
        .value("B8G8R8A8", B8G8R8A8)
        .value("B8G8R8X8", B8G8R8X8)
        .value("A8B8G8R8", A8B8G8R8)
        .value("X8B8G8R8", X8B8G8R8)
        .value("R5G6B5", R5G6B5)
        .value("R8G8B8", R8G8B8)
        .value("R8G8B8A8", R8G8B8A8)
        .value("R8G8B8X8", R8G8B8X8)
        .value("A8R8G8B8", A8R8G8B8)
        .value("X8R8G8B8", X8R8G8B8)
        .value("I8", I8)
        .value("I16", I16)
        .value("A8", A8)
        .value("YCbCr411", YCbCr411)
        .value("YCbCr422", YCbCr422)
        .value("YUYV422", YUYV422)
        .value("YCbCr420p", YCbCr420p)
        .value("YCbCrJ420p", YCbCrJ420p)
        .value("YCbCrA420p", YCbCrA420p)
        .value("BAYER8", BAYER8)
        .value("BAYER8_RGGB", BAYER8_RGGB)
        .value("BAYER8_GBRG", BAYER8_GBRG)
        .value("BAYER8_GRBG", BAYER8_GRBG)
        .value("BAYER8_BGGR", BAYER8_BGGR)
        .value("R32G32B32A32F", R32G32B32A32F)
        .value("I32F", I32F)
        .value("JPEG", JPEG)
        .export_values();

    def("getSupportedPixelFormats", &getSupportedPixelFormatsDeprecated);

    to_python_converter<Pixel32, Pixel32_to_python_tuple>();

    class_<Bitmap>("Bitmap", no_init)
        .def(init<glm::vec2, PixelFormat, UTF8String>())
        .def(init<Bitmap>())
        .def("__init__", make_constructor(createBitmapWithRect))
        .def("__init__", make_constructor(createBitmapFromFile))
        .def("blt", &Bitmap::blt)
        .def("getResized", &Bitmap_getResized)
        .def("save", &Bitmap::save)
        .def("getSize", &Bitmap_getSize)
        .def("getFormat", &Bitmap::getPixelFormat)
        .def("getPixels", &Bitmap_getPixels, Bitmap_getPixels_overloads())
        .def("setPixels", &Bitmap_setPixels, Bitmap_setPixels_overloads())
        .def("getPixel", &Bitmap::getPythonPixel)
        .def("subtract", &Bitmap::subtract)
        .def("getAvg", &Bitmap::getAvg)
        .def("getChannelAvg", &Bitmap::getChannelAvg)
        .def("getStdDev", &Bitmap::getStdDev)
        .def("getName", &Bitmap::getName, 
                return_value_policy<copy_const_reference>())
        .def("getSupportedPixelFormats", &getSupportedPixelFormats)
        .staticmethod("getSupportedPixelFormats")
    ;

    class_<ImageCache>("ImageCache", no_init)
        .add_property("capacity", ImageCache_GetCapacity, ImageCache_SetCapacity)
        .def("getNumImages", ImageCache_GetNumImages)
        .def("getMemUsed", ImageCache_GetMemUsed)
    ;

    class_<BitmapManager>("BitmapManager", no_init)
        .def("get", &BitmapManager::get,
                return_value_policy<reference_existing_object>())
        .staticmethod("get")
        .def("loadBitmap", &BitmapManager::loadBitmapPy, loadBitmap_overloads())
        .def("setNumThreads", &BitmapManager::setNumThreads)
    ;

    class_<CubicSpline, boost::noncopyable>("CubicSpline", no_init)
        .def(init<const vector<glm::vec2>&>())
        .def(init<const vector<glm::vec2>&, bool>())
        .def("interpolate", &CubicSpline::interpolate)
    ;
}
