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

#include "DistortionParams.h"

#include <../base/XMLHelper.h>
//#include <libxml/xmlstring.h>

using namespace std;

namespace avg {

    void writePoint(xmlTextWriterPtr writer, string sName, DPoint& Val)
    {
        int rc;
        rc = xmlTextWriterStartElement(writer, BAD_CAST sName.c_str());
        writeAttribute(writer, "x", Val.x);
        writeAttribute(writer, "y", Val.y);
        rc = xmlTextWriterEndElement(writer);

    }

    void writePoint3(xmlTextWriterPtr writer, string sName, DPoint3& Val)
    {
        int rc;
        rc = xmlTextWriterStartElement(writer, BAD_CAST sName.c_str());
        writeAttribute(writer, "x", Val.x);
        writeAttribute(writer, "y", Val.y);
        writeAttribute(writer, "z", Val.z);
        rc = xmlTextWriterEndElement(writer);

    }

    DistortionParams::DistortionParams() 
      : m_FilmDisplacement(0,0),
        m_FilmScale(1,1),
        m_P(0,0,0),
        m_N(0,0,1),
        m_Angle(0.0),
        m_TrapezoidFactor(0),
        m_DisplayDisplacement(0,0),
        m_DisplayScale(1,1)
    {
        m_DistortionParams.push_back(0);
        m_DistortionParams.push_back(0);
    }

    DistortionParams::DistortionParams(const DPoint& FilmDisplacement, const DPoint& FilmScale, 
            const std::vector<double>& DistortionParams, const DPoint3& P, const DPoint3& N, 
            double Angle, 
            double TrapezoidFactor, const DPoint& DisplayDisplacement, const DPoint& DisplayScale)
      : m_FilmDisplacement(FilmDisplacement),
        m_FilmScale(FilmScale),
        m_DistortionParams(DistortionParams),
        m_P(P),
        m_N(N),
        m_Angle(Angle),
        m_TrapezoidFactor(TrapezoidFactor),
        m_DisplayDisplacement(DisplayDisplacement),
        m_DisplayScale(DisplayScale)
    {
    }

    void DistortionParams::load(xmlNodePtr pParentNode)
    {
        xmlNodePtr curXmlChild = pParentNode->xmlChildrenNode;
        while (curXmlChild) {
            const char * pNodeName = (const char *)curXmlChild->name;
            if (!strcmp(pNodeName, "cameradisplacement")) {
                m_FilmDisplacement.x = getRequiredDoubleAttr(curXmlChild, "x");
                m_FilmDisplacement.y = getRequiredDoubleAttr(curXmlChild, "y");
            } else if (!strcmp(pNodeName, "camerascale")) {
                m_FilmScale.x = getRequiredDoubleAttr(curXmlChild, "x");
                m_FilmScale.y = getRequiredDoubleAttr(curXmlChild, "y");
            } else if (!strcmp(pNodeName, "distortionparams")) {
                m_DistortionParams.push_back(getRequiredDoubleAttr(curXmlChild, "p2"));
                m_DistortionParams.push_back(getRequiredDoubleAttr(curXmlChild, "p3"));
            } else if (!strcmp(pNodeName, "p")) {
                m_P.x = getRequiredDoubleAttr(curXmlChild, "x");
                m_P.y = getRequiredDoubleAttr(curXmlChild, "y");
                m_P.z = getRequiredDoubleAttr(curXmlChild, "z");
            } else if (!strcmp(pNodeName, "n")) {
                m_N.x = getRequiredDoubleAttr(curXmlChild, "x");
                m_N.y = getRequiredDoubleAttr(curXmlChild, "y");
                m_N.z = getRequiredDoubleAttr(curXmlChild, "z");
            } else if (!strcmp(pNodeName, "trapezoid")) {
                m_TrapezoidFactor = getRequiredDoubleAttr(curXmlChild, "value");
            } else if (!strcmp(pNodeName, "angle")) {
                m_Angle = getRequiredDoubleAttr(curXmlChild, "value");
            } else if (!strcmp(pNodeName, "displaydisplacement")) {
                m_DisplayDisplacement.x = getRequiredDoubleAttr(curXmlChild, "x");
                m_DisplayDisplacement.y = getRequiredDoubleAttr(curXmlChild, "y");
            } else if (!strcmp(pNodeName, "displayscale")) {
                m_DisplayScale.x = getRequiredDoubleAttr(curXmlChild, "x");
                m_DisplayScale.y = getRequiredDoubleAttr(curXmlChild, "y");
            }
            curXmlChild = curXmlChild->next;
        }
    }

    void DistortionParams::save(xmlTextWriterPtr writer)
    {
        int rc;
        rc = xmlTextWriterStartElement(writer, BAD_CAST "transform");
        writePoint(writer, "cameradisplacement", m_FilmDisplacement);
        writePoint(writer, "camerascale", m_FilmScale);
        rc = xmlTextWriterStartElement(writer, BAD_CAST "distortionparams");
        writeAttribute(writer, "p2", m_DistortionParams[0]);
        writeAttribute(writer, "p3", m_DistortionParams[1]);
        rc = xmlTextWriterEndElement(writer);
        writePoint3(writer, "p", m_P);
        writePoint3(writer, "n", m_N);
        writeSimpleXMLNode(writer, "trapezoid", m_TrapezoidFactor);
        writeSimpleXMLNode(writer, "angle", m_Angle);
        writePoint(writer, "displaydisplacement", m_DisplayDisplacement);
        writePoint(writer, "displayscale", m_DisplayScale);
        rc = xmlTextWriterEndElement(writer);
    }

}
