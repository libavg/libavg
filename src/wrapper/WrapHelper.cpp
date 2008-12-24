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

DPoint_from_python_tuple::DPoint_from_python_tuple()
{
    boost::python::converter::registry::push_back(
            &convertible,
            &construct,
            boost::python::type_id<avg::DPoint>());
}

void* DPoint_from_python_tuple::convertible(PyObject* obj_ptr)
{
    if (!PyTuple_Check(obj_ptr)) return 0;
    return obj_ptr;
}

void DPoint_from_python_tuple::construct(PyObject* obj_ptr,
        boost::python::converter::rvalue_from_python_stage1_data* data)
{
    avg::DPoint pt;
    PyObject * pEntry = PyTuple_GetItem(obj_ptr, 0);
    pt.x = PyFloat_AsDouble(pEntry);
    pEntry = PyTuple_GetItem(obj_ptr, 1);
    pt.y = PyFloat_AsDouble(pEntry);
    void* storage = (
            (boost::python::converter::rvalue_from_python_storage<avg::DPoint>*)data)->storage.bytes;
    new (storage) avg::DPoint(pt);
    data->convertible = storage;
}

IntPoint_from_python_tuple::IntPoint_from_python_tuple()
{
    boost::python::converter::registry::push_back(
            &convertible,
            &construct,
            boost::python::type_id<avg::IntPoint>());
}

void* IntPoint_from_python_tuple::convertible(PyObject* obj_ptr)
{
    if (!PyTuple_Check(obj_ptr)) return 0;
    return obj_ptr;
}

void IntPoint_from_python_tuple::construct(PyObject* obj_ptr,
        boost::python::converter::rvalue_from_python_stage1_data* data)
{
    avg::IntPoint pt;
    PyObject * pEntry = PyTuple_GetItem(obj_ptr, 0);
    pt.x = PyInt_AsLong(pEntry);
    pEntry = PyTuple_GetItem(obj_ptr, 1);
    pt.y = PyInt_AsLong(pEntry);
    void* storage = (
            (boost::python::converter::rvalue_from_python_storage<avg::IntPoint>*)data)->storage.bytes;
    new (storage) avg::IntPoint(pt);
    data->convertible = storage;
}

