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

#ifndef _WrapHelper_H_
#define _WrapHelper_H_

#include "../api.h"
#include "../base/GLMHelper.h"
#include "../base/Exception.h"
#include "../base/ILogSink.h"

#include "../player/BoostPython.h"
#include "../player/Player.h"
#include "../player/TypeRegistry.h"

#include <string>

template<typename T> const T copyObject(const T& v) { return v; }

template <typename ContainerType>
struct to_tuple
{
    static PyObject* convert(ContainerType const& a)
    {
        boost::python::list result;
        typedef typename ContainerType::const_iterator const_iter;
        for(const_iter p=a.begin();p!=a.end();p++) {
            result.append(boost::python::object(*p));
        }
        return boost::python::incref(boost::python::tuple(result).ptr());
    }

    static const PyTypeObject* get_pytype() { return &PyTuple_Type; }
};

template <typename ContainerType>
struct to_list
{
    static PyObject* convert(ContainerType const& a)
    {
        boost::python::list result;
        typedef typename ContainerType::const_iterator const_iter;
        for(const_iter p=a.begin();p!=a.end();p++) {
            result.append(boost::python::object(*p));
        }
        return boost::python::incref(result.ptr());
    }

    static const PyTypeObject* get_pytype() { return &PyList_Type; }
};

template <typename MapType>
struct to_dict
{
    static PyObject* convert(MapType const& a)
    {
        boost::python::dict result;
        typedef typename MapType::const_iterator const_iter;
        for(const_iter p=a.begin();p!=a.end();p++){
                result[p->first] = p->second;
        }
        return boost::python::incref(boost::python::dict(result).ptr());
    }
    static const PyTypeObject* get_pytype() { return &PyDict_Type; }
};

struct default_policy
{
  static bool check_convertibility_per_element() { return false; }

  template <typename ContainerType>
  static bool check_size(boost::type<ContainerType>, std::size_t /*sz*/)
  {
    return true;
  }

  template <typename ContainerType>
  static void assert_size(boost::type<ContainerType>, std::size_t /*sz*/) {}

  template <typename ContainerType>
  static void reserve(ContainerType& a, std::size_t sz) {}
};

struct fixed_size_policy
{
  static bool check_convertibility_per_element() { return true; }

  template <typename ContainerType>
  static bool check_size(boost::type<ContainerType>, std::size_t sz)
  {
    return ContainerType::size() == sz;
  }

  template <typename ContainerType>
  static void assert_size(boost::type<ContainerType>, std::size_t sz)
  {
    if (!check_size(boost::type<ContainerType>(), sz)) {
      PyErr_SetString(PyExc_RuntimeError,
        "Insufficient elements for fixed-size array.");
      boost::python::throw_error_already_set();
    }
  }

  template <typename ContainerType>
  static void reserve(ContainerType& /*a*/, std::size_t sz)
  {
    if (sz > ContainerType::size()) {
      PyErr_SetString(PyExc_RuntimeError,
        "Too many elements for fixed-size array.");
      boost::python::throw_error_already_set();
    }
  }

  template <typename ContainerType, typename ValueType>
  static void set_value(ContainerType& a, std::size_t i, ValueType const& v)
  {
    reserve(a, i+1);
    a[i] = v;
  }
};
   
struct variable_capacity_policy : default_policy
{
  template <typename ContainerType>
  static void reserve(ContainerType& a, std::size_t sz)
  {
    a.reserve(sz);
  }

  template <typename ContainerType, typename ValueType>
  static void set_value(
    ContainerType& a,
    std::size_t
#if !defined(NDEBUG)
    i
#endif
    ,
    ValueType const& v)
  {
    assert(a.size() == i);
    a.push_back(v);
  }
};

struct fixed_capacity_policy : variable_capacity_policy
{
  template <typename ContainerType>
  static bool check_size(boost::type<ContainerType>, std::size_t sz)
  {
    return ContainerType::max_size() >= sz;
  }
};

struct linked_list_policy : default_policy
{
  template <typename ContainerType, typename ValueType>
  static void
  set_value(ContainerType& a, std::size_t /*i*/, ValueType const& v)
  {
    a.push_back(v);
  }
};

struct set_policy : default_policy
{
  template <typename ContainerType, typename ValueType>
  static void
  set_value(ContainerType& a, std::size_t /*i*/, ValueType const& v)
  {
    a.insert(v);
  }
};

template <typename ContainerType, typename ConversionPolicy>
struct from_python_sequence
{
  typedef typename ContainerType::value_type container_element_type;

  from_python_sequence()
  {
    boost::python::converter::registry::push_back(
      &convertible,
      &construct,
      boost::python::type_id<ContainerType>());
  }

  static void* convertible(PyObject* obj_ptr)
  {
    if (!(   PyList_Check(obj_ptr)
          || PyTuple_Check(obj_ptr)
          || PyIter_Check(obj_ptr)
          || PyRange_Check(obj_ptr)
          || (   !PyString_Check(obj_ptr)
              && !PyUnicode_Check(obj_ptr)
              && (   obj_ptr->ob_type == 0
                  || obj_ptr->ob_type->ob_type == 0
                  || obj_ptr->ob_type->ob_type->tp_name == 0
                  || std::strcmp(
                       obj_ptr->ob_type->ob_type->tp_name,
                       "Boost.Python.class") != 0)
              && PyObject_HasAttrString(obj_ptr, "__len__")
              && PyObject_HasAttrString(obj_ptr, "__getitem__")))) return 0;
    boost::python::handle<> obj_iter(
      boost::python::allow_null(PyObject_GetIter(obj_ptr)));
    if (!obj_iter.get()) { // must be convertible to an iterator
      PyErr_Clear();
      return 0;
    }
    if (ConversionPolicy::check_convertibility_per_element()) {
      int obj_size = int(PyObject_Length(obj_ptr));
      if (obj_size < 0) { // must be a measurable sequence
        PyErr_Clear();
        return 0;
      }
      if (!ConversionPolicy::check_size(
        boost::type<ContainerType>(), obj_size)) return 0;
      bool is_range = PyRange_Check(obj_ptr);
      std::size_t i=0;
      if (!all_elements_convertible(obj_iter, is_range, i)) return 0;
      if (!is_range) assert(i == (std::size_t)obj_size);
    }
    return obj_ptr;
  }

  // This loop factored out by Achim Domma to avoid Visual C++
  // Internal Compiler Error.
  static bool
  all_elements_convertible(
    boost::python::handle<>& obj_iter,
    bool is_range,
    std::size_t& i)
  {
    for(;;i++) {
      boost::python::handle<> py_elem_hdl(
        boost::python::allow_null(PyIter_Next(obj_iter.get())));
      if (PyErr_Occurred()) {
        PyErr_Clear();
        return false;
      }
      if (!py_elem_hdl.get()) break; // end of iteration
      boost::python::object py_elem_obj(py_elem_hdl);
      boost::python::extract<container_element_type>
        elem_proxy(py_elem_obj);
      if (!elem_proxy.check()) return false;
      if (is_range) break; // in a range all elements are of the same type
    }
    return true;
  }

  static void construct(
    PyObject* obj_ptr,
    boost::python::converter::rvalue_from_python_stage1_data* data)
  {
    boost::python::handle<> obj_iter(PyObject_GetIter(obj_ptr));
    void* storage = (
      (boost::python::converter::rvalue_from_python_storage<ContainerType>*)
        data)->storage.bytes;
    new (storage) ContainerType();
    data->convertible = storage;
    ContainerType& result = *((ContainerType*)storage);
    std::size_t i=0;
    for(;;i++) {
      boost::python::handle<> py_elem_hdl(
        boost::python::allow_null(PyIter_Next(obj_iter.get())));
      if (PyErr_Occurred()) boost::python::throw_error_already_set();
      if (!py_elem_hdl.get()) break; // end of iteration
      boost::python::object py_elem_obj(py_elem_hdl);
      boost::python::extract<container_element_type> elem_proxy(py_elem_obj);
      ConversionPolicy::set_value(result, i, elem_proxy());
    }
    ConversionPolicy::assert_size(boost::type<ContainerType>(), i);
  }
};

template<class T>
float deprecatedGet(T& node)
{
    throw avg::Exception(AVG_ERR_DEPRECATED, "Attribute has been removed from libavg.");
}

template<class T>
void deprecatedSet(T& node, float d)
{
    throw avg::Exception(AVG_ERR_DEPRECATED, "Attribute has been removed from libavg.");
}

namespace Vec2Helper
{
    int len(const glm::vec2&);
    float getX(const glm::vec2& pt);
    float getY(const glm::vec2& pt);
    void setX(glm::vec2& pt, float val);
    void setY(glm::vec2& pt, float val);
    void checkItemRange(int i);
    float getItem(const glm::vec2& pt, int i);
    void setItem(glm::vec2& pt, int i, float val);
    std::string str(const glm::vec2& pt);
    std::string repr(const glm::vec2& pt);
    long getHash(const glm::vec2& pt);
    glm::vec2 safeGetNormalized(const glm::vec2& pt);
    float getNorm(const glm::vec2& pt);
    float vecAngle(const glm::vec2& pt1, const glm::vec2& pt2);
}

class ConstVec2: public glm::vec2
{
public:
    ConstVec2();
    ConstVec2(const glm::vec2& other);
    glm::vec2 toVec2() const;
    //operator glm::vec2() const;
};

AVG_API void checkEmptyArgs(const boost::python::tuple &args, int numArgs=1);

template<const char * pszType> 
avg::ExportedObjectPtr createExportedObject(const boost::python::tuple &args,
        const boost::python::dict &attrs)
{
    checkEmptyArgs(args);
    return avg::TypeRegistry::get()->createObject(pszType, attrs);
}

template<const char * pszType> 
avg::NodePtr createNode(const boost::python::tuple &args,
        const boost::python::dict &attrs)
{
    checkEmptyArgs(args);
    return avg::Player::get()->createNode(pszType, attrs, args[0]);
};

template <typename T>struct Exception_to_python_exception
{
    static PyObject* convert (const T& ex)
    {
        PyObject *arglist = boost::python::incref(
                Py_BuildValue("(s)", ex.what()));
        return boost::python::incref(
                PyObject_CallObject(PyExc_RuntimeError, arglist));
    }
};


template<typename T> struct ExceptionTranslator
{
public:
    void operator()(const T& ex) const
    {
      PyErr_SetString(m_PyExcept, ex.what());
    }

    ExceptionTranslator(PyObject* py_except): m_PyExcept(py_except)
    {
      boost::python::register_exception_translator<T>(*this);
    }

    ExceptionTranslator(const ExceptionTranslator& other):m_PyExcept(other.m_PyExcept)
    {
    }

  private:

    PyObject* m_PyExcept;

};

template <typename T> void translateException(PyObject* e) {
  ExceptionTranslator<T> my_translator(e);
}

void exportMessages(boost::python::object& nodeClass, const std::string& sClassName);

void addPythonLogger(PyObject * self, PyObject * pyLogger);
void removePythonLogger(PyObject * self, PyObject * pyLogger);

void pytrace(PyObject * self, const avg::category_t& category, const avg::UTF8String& sMsg,
        avg::severity_t severity);

#endif
