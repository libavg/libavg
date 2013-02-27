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

#ifndef _ExportedObject_H_
#define _ExportedObject_H_

#include "../api.h"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

namespace avg {

class TypeDefinition;
class ArgList;

class ExportedObject;
typedef boost::shared_ptr<ExportedObject> ExportedObjectPtr;

class AVG_API ExportedObject: public boost::enable_shared_from_this<ExportedObject>
{
    public:
        ExportedObject();
        ExportedObject(const ExportedObject& other);
        virtual ~ExportedObject()=0;

        void registerInstance(PyObject* pSelf);
        ExportedObjectPtr getSharedThis();

        template<class Type>
        static ExportedObjectPtr buildObject(const ArgList& Args)
        {
            return ExportedObjectPtr(new Type(Args));
        }
        virtual void setTypeInfo(const TypeDefinition * pDefinition);
        
        virtual void setArgs(const ArgList& args) {};
        std::string getTypeStr() const;
        virtual const TypeDefinition* getDefinition() const;

        bool operator ==(const ExportedObject& other) const;
        bool operator !=(const ExportedObject& other) const;
        long getHash() const;

    private:
        const TypeDefinition* m_pDefinition;
        PyObject* m_pSelf;
};

}

#endif
