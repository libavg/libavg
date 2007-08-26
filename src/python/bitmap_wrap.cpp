//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "../graphics/Bitmap.h"

#include "../base/Point.h"

#include <boost/python.hpp>

using namespace boost::python;
using namespace avg;

struct IntPoint_to_python_tuple
{
    static PyObject* convert (IntPoint pt)
    {
        return boost::python::incref(make_tuple(pt.x, pt.y).ptr());
    }
};

struct DPoint_from_python_tuple
{
    DPoint_from_python_tuple()
    {
        boost::python::converter::registry::push_back(
                &convertible,
                &construct,
                boost::python::type_id<DPoint>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (!PyTuple_Check(obj_ptr)) return 0;
        return obj_ptr;
    }

    static void construct(PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        DPoint pt;
        PyObject * pEntry = PyTuple_GetItem(obj_ptr, 0);
        pt.x = PyFloat_AsDouble(pEntry);
        pEntry = PyTuple_GetItem(obj_ptr, 1);
        pt.y = PyFloat_AsDouble(pEntry);
        void* storage = (
                (converter::rvalue_from_python_storage<DPoint>*)data)->storage.bytes;
        new (storage) DPoint(pt);
        data->convertible = storage;
    }
};

void export_bitmap()
{
    to_python_converter<IntPoint, IntPoint_to_python_tuple>();
    DPoint_from_python_tuple();
    
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
        .value("YCbCr422", YCbCr422)
        .export_values();
    
    // TODO: Change this so it uses custom conversion operators like IntPoint.
    // See http://www.boost.org/libs/python/doc/v2/faq.html#custom_string
    class_<DPoint>("Point",
            "A point is the basic coordinate in avg. Points are usually expressed\n"
            "in floating-point coordinates.")
        .def(init<double, double>())
        .def(init<DPoint>())
        .def_readwrite("x", &DPoint::x)
        .def_readwrite("y", &DPoint::y)
    ;

    class_<Bitmap>("Bitmap",
            "Class representing a rectangular set of pixels. Bitmaps can be obtained\n"
            "from any RasterNode. For nodes of type Image, the current bitmap can be\n"
            "set as well.",
            no_init)
        .def(init<IntPoint, PixelFormat, std::string>())
        .def(init<Bitmap>())
        .def(init<std::string>())
        .def("save", &Bitmap::save,
                "save(filename)\n"
                "Writes the image to a file. File format is determined using the\n"
                "extension. Any file format specified by ImageMagick \n"
                "(U{http://www.imagemagick.org}) can be used.")
        .def("getSize", &Bitmap::getSize,
                "getSize()\n\n"
                "Returns the size of the image in pixels.")
        .def("getFormat", &Bitmap::getPixelFormat, 
                "getFormat()\n"
                "Returns the layout of the pixels in the bitmap.\n"
                "Possible return values are B5G6R5, B8G8R8, B8G8R8A8, B8G8R8X8,\n"
                "A8B8G8R8, X8B8G8R8, R5G6B5, R8G8B8, R8G8B8A8, R8G8B8X8, A8R8G8B8,\n"
                "X8R8G8B8, I8 and YCbCr422.")
        .def("getPixels", &Bitmap::getPixelsAsString, 
                "getPixels()\n"
                "Returns the raw pixel data in the bitmap as a python string. This\n"
                "method can be used to interface to the python imaging library PIL\n"
                "(U{http://www.pythonware.com/products/pil/}).")
        .def("setPixels", &Bitmap::setPixelsFromString,
                "setPixels(pixels)\n\n"
                "Changes the raw pixel data in the bitmap. Doesn't change dimensions \n"
                "or pixel format. Can be used to interface to the python imaging\n"
                "library PIL (U{http://www.pythonware.com/products/pil/}).\n"
                "@param pixels: Image data as a python string.")
        .def("subtract", &Bitmap::subtract, 
                "subtract(otherbitmap)\n")
        .def("getName", &Bitmap::getName, 
                return_value_policy<copy_const_reference>(),
                "getName() -> string\n\n")
    ;
    
}
