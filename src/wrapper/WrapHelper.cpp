//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/MathHelper.h"
#include "../base/ObjectCounter.h"

#include "../player/PythonLogSink.h"
#include "../player/PublisherDefinitionRegistry.h"

#include <boost/version.hpp>

using namespace avg;
using namespace std;
using namespace boost::python;

namespace Vec2Helper
{
    int len(const glm::vec2&) 
    {
        return 2;
    }

    float getX(const glm::vec2& pt)
    {
        return pt.x;
    }

    float getY(const glm::vec2& pt)
    {
        return pt.y;
    }

    void setX(glm::vec2& pt, float val)
    {
        pt.x = val;
    }

    void setY(glm::vec2& pt, float val)
    {
        pt.y = val;
    }

    void checkItemRange(int i) {
        if (i != 0 && i != 1) {
            throw std::out_of_range("Index out of range for Point2D. Must be 0 or 1.");
        }
    }

    float getItem(const glm::vec2& pt, int i)
    {
        checkItemRange(i);
        if (i==0) {
            return pt.x;
        } else {
            return pt.y;
        }
    }

    void setItem(glm::vec2& pt, int i, float val)
    {
        checkItemRange(i);
        if (i==0) {
            pt.x = val;
        } else {
            pt.y = val;
        }
    }

    string str(const glm::vec2& pt)
    {
        stringstream st;
        st << "(" << pt.x << "," << pt.y << ")";
        return st.str();
    }

    string repr(const glm::vec2& pt)
    {
        stringstream st;
        st << "avg.Point2D(" << pt.x << "," << pt.y << ")";
        return st.str();
    }

    long getHash(const glm::vec2& pt)
    {
        // Wild guess at what could constitute a good hash function.
        // Will generate very bad hashes if most values are in a range < 0.1,
        // but this is meant for pixel values anyway, right? ;-).
        return long(pt.x*42+pt.y*23);
    }
    
    glm::vec2 safeGetNormalized(const glm::vec2& pt)
    {
        if (pt.x==0 && pt.y==0) {
            throw Exception(AVG_ERR_OUT_OF_RANGE, "Can't normalize (0,0).");
        } else {
            float invNorm = 1/sqrt(pt.x*pt.x+pt.y*pt.y);
            return glm::vec2(pt.x*invNorm, pt.y*invNorm);
        }
    }

    float getNorm(const glm::vec2& pt)
    {
        return glm::length(pt);
    }
    
    float vecAngle(const glm::vec2& pt1, const glm::vec2& pt2)
    {
        float angle = fmod((atan2(pt1.y, pt1.x) - atan2(pt2.y, pt2.x)), float(2*M_PI));
        if (angle < 0) {
            angle += 2*M_PI;
        }
        return angle;
    }
}

// The ConstVec2 stuff is there so that vec2 attributes behave sensibly. That is,
// node.pos.x = 30 causes an error instead of failing silently.
ConstVec2::ConstVec2()
{
}

ConstVec2::ConstVec2(const glm::vec2& other)
{
    x = other.x;
    y = other.y;
}

glm::vec2 ConstVec2::toVec2() const
{
    return glm::vec2(x,y);
}

void checkEmptyArgs(const boost::python::tuple &args, int numArgs)
{
    if (boost::python::len(args) != numArgs) {
        throw avg::Exception(AVG_ERR_INVALID_ARGS, 
                "Nodes must be constructed using named parameters. Positional parameters are not supported.");
    }
}

template<class VEC2>
struct Vec2_to_python_tuple
{
    static PyObject* convert (VEC2 v)
    {
        return boost::python::incref(boost::python::make_tuple(v.x, v.y).ptr());
    }
};

template<class VEC3>
struct Vec3_to_python_tuple
{
    static PyObject* convert (VEC3 v)
    {
        return boost::python::incref(boost::python::make_tuple(v.x, v.y, v.z).ptr());
    }
};

template<class VEC4>
struct Vec4_to_python_tuple
{
    static PyObject* convert (VEC4 v)
    {
        return boost::python::incref(boost::python::make_tuple(v.x, v.y, v.z, v.w).ptr());
    }
};

template<class VEC2, class ATTR>
struct vec2_from_python
{
    vec2_from_python() 
    {
        boost::python::converter::registry::push_back(
                &convertible, &construct, boost::python::type_id<VEC2>());
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
        VEC2 pt;
        PyObject * pEntry = PySequence_GetItem(obj_ptr, 0);
        pt.x = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        pEntry = PySequence_GetItem(obj_ptr, 1);
        pt.y = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<VEC2>*)data)
                    ->storage.bytes;
        new (storage) VEC2(pt);
        data->convertible = storage;
    }
};

template<class VEC3, class ATTR>
struct vec3_from_python
{
    vec3_from_python() 
    {
        boost::python::converter::registry::push_back(
                &convertible, &construct, boost::python::type_id<VEC3>());
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
        VEC3 t;
        PyObject * pEntry = PySequence_GetItem(obj_ptr, 0);
        t.x = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        pEntry = PySequence_GetItem(obj_ptr, 1);
        t.y = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        pEntry = PySequence_GetItem(obj_ptr, 2);
        t.z = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<VEC3>*)
                        data)->storage.bytes;
        new (storage) VEC3(t);
        data->convertible = storage;
    }
};

template<class VEC4, class ATTR>
struct vec4_from_python
{
    vec4_from_python() 
    {
        boost::python::converter::registry::push_back(
                &convertible, &construct, boost::python::type_id<VEC4>());
    }
    
    static void* convertible(PyObject* obj_ptr)
    {
        if (!PySequence_Check(obj_ptr)) {
            return 0;
        }
        if (PySequence_Size(obj_ptr) != 4) {
            return 0;
        }
        return obj_ptr;
    }

    static void construct(PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        VEC4 t;
        PyObject * pEntry = PySequence_GetItem(obj_ptr, 0);
        t.x = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        pEntry = PySequence_GetItem(obj_ptr, 1);
        t.y = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        pEntry = PySequence_GetItem(obj_ptr, 2);
        t.z = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        pEntry = PySequence_GetItem(obj_ptr, 3);
        t.w = (ATTR)PyFloat_AsDouble(pEntry);
        Py_DECREF(pEntry);
        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<VEC4>*)
                        data)->storage.bytes;
        new (storage) VEC4(t);
        data->convertible = storage;
    }
};

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
        PyObject * pPyUTF8 = PyUnicode_AsUTF8String(obj_ptr);
        char * psz = PyString_AsString(pPyUTF8);
        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<UTF8String>*)data)
                        ->storage.bytes;
        new (storage) UTF8String(psz);
        data->convertible = storage;
        Py_DECREF(pPyUTF8);
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
        const char * psz = PyString_AsString(obj_ptr);
        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<UTF8String>*)data)
                        ->storage.bytes;
        new (storage) UTF8String(psz);
        data->convertible = storage;
    }
};

void exportMessages(object& nodeClass, const string& sClassName)
{
    PublisherDefinitionPtr pPubDef = PublisherDefinitionRegistry::get()
            ->getDefinition(sClassName);
    const vector<MessageID>& messageIDs = pPubDef->getMessageIDs();
    for (unsigned i=0; i<messageIDs.size(); ++i) {
        string sName = messageIDs[i].m_sName;
        nodeClass.attr(sName.c_str()) = messageIDs[i];
    }
};

struct type_info_to_string{
    static PyObject* convert(const std::type_info& info)
    {
        boost::python::object result(ObjectCounter::get()->demangle(info.name()));
        return boost::python::incref(result.ptr());
    }
};


void export_base()
{
    // Exceptions

    translateException<exception>(PyExc_RuntimeError);
    translateException<out_of_range>(PyExc_IndexError);
    translateException<Exception>(PyExc_RuntimeError);
    to_python_converter< exception, Exception_to_python_exception<exception> >();
    to_python_converter< Exception, Exception_to_python_exception<Exception> >();
   
    // vec2
    to_python_converter<IntPoint, Vec2_to_python_tuple<IntPoint> >();
    vec2_from_python<IntPoint, int>();
    vec2_from_python<glm::vec2, float>();
    vec2_from_python<ConstVec2, float>();
   
    // vector<vec2>
    to_python_converter<vector<glm::vec2>, to_list<vector<glm::vec2> > >();    
    from_python_sequence<vector<IntPoint>, variable_capacity_policy>();
    from_python_sequence<vector<glm::vec2>, variable_capacity_policy>();

    // vec3
    to_python_converter<glm::ivec3, Vec3_to_python_tuple<glm::ivec3> >();
    to_python_converter<glm::vec3, Vec3_to_python_tuple<glm::vec3> >();
    vec3_from_python<glm::ivec3, int>();
    vec3_from_python<glm::vec3, float>();
    
    // vec4
    to_python_converter<glm::ivec4, Vec4_to_python_tuple<glm::ivec4> >();
    to_python_converter<glm::vec4, Vec4_to_python_tuple<glm::vec4> >();
    vec4_from_python<glm::ivec4, int>();
    vec4_from_python<glm::vec4, float>();
    
    // vector<vec3>
    to_python_converter<vector<glm::ivec3>, to_list<vector<glm::ivec3> > >();    
    to_python_converter<vector<glm::vec3>, to_list<vector<glm::vec3> > >();    
    from_python_sequence<vector<glm::ivec3>, variable_capacity_policy>();
    from_python_sequence<vector<glm::vec3>, variable_capacity_policy>();

    // string
    to_python_converter<UTF8String, UTF8String_to_unicode>();
    UTF8String_from_unicode();
    UTF8String_from_string();

    to_python_converter<vector<string>, to_list<vector<string> > >();    
    from_python_sequence<vector<string>, variable_capacity_policy>();
  
    from_python_sequence<vector<float>, variable_capacity_policy>();
    from_python_sequence<vector<int>, variable_capacity_policy>();

    to_python_converter<std::type_info, type_info_to_string>();
    //Maps
    to_python_converter<TypeMap, to_dict<TypeMap> >();
    to_python_converter<CatToSeverityMap, to_dict<CatToSeverityMap> >();
}

namespace {
    std::map<PyObject *, LogSinkPtr> m_pyObjectMap;
}

void addPythonLogger(PyObject * self, PyObject * pyLogger)
{
    Logger * logger = Logger::get();
    LogSinkPtr logSink(new PythonLogSink(pyLogger));
    logger->addLogSink(logSink);
    m_pyObjectMap[pyLogger] = logSink;
}

void removePythonLogger(PyObject * self, PyObject * pyLogger)
{
    Logger* logger = Logger::get();
    std::map<PyObject *, LogSinkPtr>::iterator it;
    it = m_pyObjectMap.find(pyLogger);
    if( it !=m_pyObjectMap.end() ){
        logger->removeLogSink(it->second);
        m_pyObjectMap.erase(it);
    }
}

void pytrace(PyObject * self, const avg::category_t& category, const UTF8String& sMsg,
        avg::severity_t severity)
{
    avgDeprecationWarning(string("1.8"), "logger.trace",
            "any of the logging convenience functions");
    Logger::get()->trace(sMsg, category, severity);
}

