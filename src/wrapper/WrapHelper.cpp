//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "../base/Exception.h"

#include <boost/version.hpp>

using namespace avg;
using namespace std;
using namespace boost::python;

namespace DPointHelper
{
    int len(const DPoint&) 
    {
        return 2;
    }

    double getX(const DPoint& pt)
    {
        return pt.x;
    }

    double getY(const DPoint& pt)
    {
        return pt.y;
    }

    void setX(DPoint& pt, double val)
    {
        pt.x = val;
    }

    void setY(DPoint& pt, double val)
    {
        pt.y = val;
    }

    void checkItemRange(int i) {
        if (i != 0 && i != 1) {
            throw std::out_of_range("Index out of range for Point2D. Must be 0 or 1.");
        }
    }

    double getItem(const DPoint& pt, int i)
    {
        checkItemRange(i);
        if (i==0) {
            return pt.x;
        } else {
            return pt.y;
        }
    }

    void setItem(DPoint& pt, int i, double val)
    {
        checkItemRange(i);
        if (i==0) {
            pt.x = val;
        } else {
            pt.y = val;
        }
    }

    string str(const DPoint& pt)
    {
        stringstream st;
        st << "(" << pt.x << "," << pt.y << ")";
        return st.str();
    }

    string repr(const DPoint& pt)
    {
        stringstream st;
        st << "avg.Point2D(" << pt.x << "," << pt.y << ")";
        return st.str();
    }

    long getHash(const DPoint& pt)
    {
        // Wild guess at what could constitute a good hash function.
        // Will generate very bad hashes if most values are in a range < 0.1,
        // but this is meant for pixel values anyway, right? ;-).
        return long(pt.x*42+pt.y*23);
    }
}

// The ConstDPoint stuff is there so that DPoint attributes behave sensibly. That is,
// node.pos.x = 30 causes an error instead of failing silently.
ConstDPoint::ConstDPoint()
{
}

ConstDPoint::ConstDPoint(const DPoint& other)
{
    x = other.x;
    y = other.y;
}

ConstDPoint::operator DPoint() const
{
    return DPoint(x,y);
}

void checkEmptyArgs(const boost::python::tuple &args, int numArgs)
{
    if (boost::python::len(args) != numArgs) {
        throw avg::Exception(AVG_ERR_INVALID_ARGS, 
                "Nodes must be constructed using named parameters. Positional parameters are not supported.");
    }
}

template<class NUM>
struct Point_to_python_tuple
{
    static PyObject* convert (avg::Point<NUM> pt)
    {
        return boost::python::incref(boost::python::make_tuple(pt.x, pt.y).ptr());
    }
};

struct Exception_to_python_exception
{
    static PyObject* convert (avg::Exception ex)
    {
        PyObject *arglist = boost::python::incref(Py_BuildValue("(s)", ex.getStr().c_str()));
        
        return boost::python::incref(
                PyObject_CallObject(PyExc_RuntimeError, arglist));
    }
};

template<class NUM>
struct Triple_to_python_tuple
{
    static PyObject* convert (avg::Triple<NUM> t)
    {
        return boost::python::incref(boost::python::make_tuple(t.x,t.y,t.z).ptr());
    }
};

template<class POINT, class ATTR>
struct point_from_python
{
    point_from_python() 
    {
        boost::python::converter::registry::push_back(
                &convertible, &construct, boost::python::type_id<POINT>());
    }
    
    static void* convertible(PyObject* obj_ptr)
    {
        // Using PySequence_Check here causes infinite recursion.
        if (!PyTuple_Check(obj_ptr) && !PyList_Check(obj_ptr)) {
            return 0;
        }
        if (PySequence_Size(obj_ptr) != 2) {
            return 0;
        }
        return obj_ptr;
    }

    static void construct(PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        POINT pt;
        PyObject * pEntry = PySequence_GetItem(obj_ptr, 0);
        pt.x = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        pEntry = PySequence_GetItem(obj_ptr, 1);
        pt.y = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<POINT>*)data)
                    ->storage.bytes;
        new (storage) POINT(pt);
        data->convertible = storage;
    }
};

template<class NUM>
struct triple_from_python
{
    triple_from_python() 
    {
        boost::python::converter::registry::push_back(
                &convertible, &construct, boost::python::type_id<Triple<NUM> >());
    }
    
    static void* convertible(PyObject* obj_ptr)
    {
        if (!PySequence_Check(obj_ptr)) {
            return 0;
        }
        if (PySequence_Size(obj_ptr) != 3) {
            return 0;
        }
        return obj_ptr;
    }

    static void construct(PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        avg::Triple<NUM> t;
        PyObject * pEntry = PySequence_GetItem(obj_ptr, 0);
        t.x = (NUM)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        pEntry = PySequence_GetItem(obj_ptr, 1);
        t.y = (NUM)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        pEntry = PySequence_GetItem(obj_ptr, 2);
        t.z = (NUM)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<Triple<NUM> >*)
                        data)->storage.bytes;
        new (storage) Triple<NUM>(t);
        data->convertible = storage;
    }
};

void exception_translator(Exception const & e) 
{
    PyErr_SetString(PyExc_RuntimeError, e.getStr().c_str());
}

struct UTF8String_to_unicode
{
    static PyObject *convert(const UTF8String & s)
    {
        const char * pStr = s.c_str();
        return PyUnicode_DecodeUTF8(pStr, strlen(pStr), "ignore");
    }
};

struct UTF8String_from_unicode
{
    UTF8String_from_unicode()
    {
        boost::python::converter::registry::push_back(
                &convertible,
                &construct,
                boost::python::type_id<UTF8String>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (!PyUnicode_Check(obj_ptr)) return 0;
        return obj_ptr;
    }

    static void construct(PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        UTF8String s;
        PyObject * pPyUTF8 = PyUnicode_AsUTF8String(obj_ptr);
        char * psz = PyString_AsString(pPyUTF8);
        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<UTF8String>*)data)
                        ->storage.bytes;
        new (storage) UTF8String(psz);
        data->convertible = storage;
    }
};

struct UTF8String_from_string
{
    UTF8String_from_string()
    {
        boost::python::converter::registry::push_back(
                &convertible,
                &construct,
                boost::python::type_id<UTF8String>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (!PyString_Check(obj_ptr)) return 0;
        return obj_ptr;
    }

    static void construct(PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        UTF8String s;
        char * psz = PyString_AsString(obj_ptr);
        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<UTF8String>*)data)
                        ->storage.bytes;
        new (storage) UTF8String(psz);
        data->convertible = storage;
    }
};

void export_base()
{
#if (BOOST_VERSION / 100000) > 1 || ((BOOST_VERSION / 100) % 1000) >= 33
    register_exception_translator<Exception>(exception_translator);
#endif
    to_python_converter<Exception, Exception_to_python_exception>();
    to_python_converter<IntPoint, Point_to_python_tuple<int> >();
    to_python_converter<DTriple, Triple_to_python_tuple<double> >();
    point_from_python<DPoint, double>();
    point_from_python<ConstDPoint, double>();
    point_from_python<IntPoint, int>();
    
    triple_from_python<double>();
    triple_from_python<int>();
    
    to_python_converter<vector<DPoint>, to_list<vector<DPoint> > >();    
    to_python_converter<vector<string>, to_list<vector<string> > >();    
   
    from_python_sequence<vector<DPoint>, variable_capacity_policy>();
    from_python_sequence<vector<IntPoint>, variable_capacity_policy>();
    from_python_sequence<vector<string>, variable_capacity_policy>();
  
    from_python_sequence<vector<IntTriple>, variable_capacity_policy>();
    from_python_sequence<vector<DTriple>, variable_capacity_policy>();
    from_python_sequence<vector<double>, variable_capacity_policy>();
    
    to_python_converter<UTF8String, UTF8String_to_unicode>();
    UTF8String_from_unicode();
    UTF8String_from_string();
}

