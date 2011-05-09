//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "../graphics/Bitmap.h"

#include "../base/Point.h"

#include <vector>
#include <sstream>

using namespace boost::python;
using namespace std;
using namespace avg;

template<class POINT>
class_<POINT> export_point(const string& sName)
{
    return class_<POINT>(sName.c_str(), no_init)
        .def("__len__", &DPointHelper::len)
        .def("__getitem__", &DPointHelper::getItem)
        .def("__str__", &DPointHelper::str)
        .def("__repr__", &DPointHelper::repr)
        .def("__hash__", &DPointHelper::getHash)
        .def("getNormalized", &DPoint::safeGetNormalized)
        .def("getNorm", &DPoint::getNorm)
        .def("getRotated", &DPoint::getRotated)
        .def("getRotated", &DPoint::getRotatedPivot)
        .def("isNaN", &DPoint::isNaN)
        .def("isInf", &DPoint::isInf)
        .def(self == self)
        .def(self != self)
        .def(-self)
        .def(self + self)
        .def(self - self)
        .def(float() * self)
        .def(self * float())
        .def(self / float())
        .def("fromPolar", &DPoint::fromPolar)
        .staticmethod("fromPolar")
        .def("getAngle", &DPoint::getAngle)
    ;
}

struct Pixel32_to_python_tuple
{
    static PyObject* convert (avg::Pixel32 px)
    {
        return boost::python::incref(boost::python::make_tuple(
                px.getR(), px.getG(), px.getB(), px.getA()).ptr());
    }
};

ConstDPoint Bitmap_getSize(Bitmap* This)
{
    return (DPoint)(This->getSize());
}

DPoint* createPoint()
{
    return new DPoint(0,0);
}

void export_bitmap()
{
    export_point<DPoint>("Point2D")
        .def("__init__", make_constructor(createPoint))
        .def(init<double, double>())
        .def(init<const DPoint&>())
        .def("__setitem__", &DPointHelper::setItem)
        .add_property("x", &DPointHelper::getX, &DPointHelper::setX,"")
        .add_property("y", &DPointHelper::getY, &DPointHelper::setY,"")
    ;
    export_point<ConstDPoint>("ConstPoint2D")
        .add_property("x", &DPointHelper::getX, "")
        .add_property("y", &DPointHelper::getY, "")
    ;

    implicitly_convertible<ConstDPoint, DPoint>();
    implicitly_convertible<DPoint, ConstDPoint>();

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
        .value("YCbCrA420p", YCbCrA420p)
        .value("BAYER8", BAYER8)
        .value("BAYER8_RGGB", BAYER8_RGGB)
        .value("BAYER8_GBRG", BAYER8_GBRG)
        .value("BAYER8_GRBG", BAYER8_GRBG)
        .value("BAYER8_BGGR", BAYER8_BGGR)
        .value("R32G32B32A32F", R32G32B32A32F)
        .value("I32F", I32F)
        .export_values();

    to_python_converter<Pixel32, Pixel32_to_python_tuple>();

    class_<Bitmap, boost::shared_ptr<Bitmap> >("Bitmap", no_init)
        .def(init<DPoint, PixelFormat, UTF8String>())
        .def(init<Bitmap>())
        .def(init<UTF8String>())
        .def("save", &Bitmap::save)
        .def("getSize", &Bitmap_getSize)
        .def("getFormat", &Bitmap::getPixelFormat)
        .def("getPixels", &Bitmap::getPixelsAsString)
        .def("setPixels", &Bitmap::setPixelsFromString)
        .def("getPixel", &Bitmap::getPythonPixel)
        .def("subtract", &Bitmap::subtract,
                return_value_policy<manage_new_object>())
        .def("getAvg", &Bitmap::getAvg)
        .def("getChannelAvg", &Bitmap::getChannelAvg)
        .def("getStdDev", &Bitmap::getStdDev)
        .def("getName", &Bitmap::getName, 
                return_value_policy<copy_const_reference>())
    ;
    
}
