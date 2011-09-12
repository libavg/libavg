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

#include "SVGElement.h"

#include "../base/StringHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"

using namespace std;

namespace avg {

SVGElement::SVGElement(RsvgHandle* pRSVG, const UTF8String& sFilename, 
        const UTF8String& sElementID, bool bUnescapeIllustratorIDs)
{
    m_sUnescapedID = unescapeID(pRSVG, sFilename, sElementID, bUnescapeIllustratorIDs); 
    
    RsvgPositionData pos;
    rsvg_handle_get_position_sub(pRSVG, &pos, m_sUnescapedID.c_str());
    m_Pos = DPoint(pos.x, pos.y);
    
    RsvgDimensionData dim;
    rsvg_handle_get_dimensions_sub(pRSVG, &dim, m_sUnescapedID.c_str());
    m_Size = DPoint(dim.width+1, dim.height+1);
}

const UTF8String& SVGElement::getUnescapedID() const
{
    return m_sUnescapedID;
}

const DPoint& SVGElement::getPos() const
{
    return m_Pos;
}

const DPoint& SVGElement::getSize() const
{
    return m_Size;
}

UTF8String SVGElement::unescapeID(RsvgHandle* pRSVG, const UTF8String& sFilename,
        const UTF8String& sElementID, bool bUnescapeIllustratorIDs)
{
    UTF8String sResult(string("#") + sElementID);
    if (bUnescapeIllustratorIDs) {
        vector<string> sPossibleIDs;
        sPossibleIDs.push_back(sResult);
        string::size_type pos = sResult.find("_");
        while (pos != UTF8String::npos) {
            sResult.replace(pos, 1, "_x5F_");
            pos = sResult.find("_", pos+5);
        }
        // Illustrator adds suffixes to IDs to get rid of duplicates. Even after the
        // duplicates are removed, the suffixes remain :-(.
        // We handle two cases here: 
        // 1) If there is only one version with a suffix, we take that version.
        // 2) If there are duplicate IDs, we throw an error.
        sPossibleIDs.push_back(sResult);
        for (int i=1; i<10; ++i) {
            string sTempID = sResult + "_" + toString(i) + "_";
            sPossibleIDs.push_back(sTempID);
        }
        int numFound = 0;
        for (int i=0; i<10; ++i) {
            string sTempID = sPossibleIDs[i];
            if (rsvg_handle_has_sub(pRSVG, sTempID.c_str())) {
                sResult = sTempID;
                numFound += 1;
            }
        }
        if (numFound == 0) {
            throwIDNotFound(sFilename, sElementID);
        }
        if (numFound > 1) {
            AVG_TRACE(Logger::WARNING,
                    "svg file '" << sFilename << 
                    "' has more than one element with id '" << sElementID << "'.");
        }
    } else {
        if (!rsvg_handle_has_sub(pRSVG, sResult.c_str())) {
            throwIDNotFound(sFilename, sElementID);
        }
    }
    return sResult;
}

void SVGElement::throwIDNotFound(const UTF8String& sFilename, 
        const UTF8String& sElementID)
{
    throw Exception(AVG_ERR_INVALID_ARGS, 
            string("svg file '") + sFilename + "' does not have an element with id '" + 
            sElementID + "'.");
}

}
