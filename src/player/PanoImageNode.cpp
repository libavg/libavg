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

#include "PanoImageNode.h"
#include "NodeDefinition.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/ProfilingZone.h"
#include "../base/ScopeTimer.h"
#include "../base/Exception.h"
#include "../base/XMLHelper.h"

#include "../graphics/OGLHelper.h"

#include <iostream>
#include <sstream>
#include <math.h>

using namespace std;

const int TEX_WIDTH = 64;

namespace avg {

NodeDefinition PanoImageNode::createDefinition()
{
    return NodeDefinition("panoimage", Node::buildNode<PanoImageNode>)
        .extendDefinition(AreaNode::createDefinition())
        .addArg(Arg<UTF8String>("href", "", false, offsetof(PanoImageNode, m_href)))
        .addArg(Arg<float>("sensorwidth", 1.0, false, offsetof(PanoImageNode, m_SensorWidth)))
        .addArg(Arg<float>("sensorheight", 1.0, false, offsetof(PanoImageNode, m_SensorHeight)))
        .addArg(Arg<float>("focallength", 10.0, false, offsetof(PanoImageNode, m_FocalLength)))
        .addArg(Arg<float>("rotation", -1.0, false, offsetof(PanoImageNode, m_Rotation)));
}

PanoImageNode::PanoImageNode (const ArgList& Args)
{
    Args.setMembers(this);
    m_pBmp = BitmapPtr(new Bitmap(IntPoint(1,1), R8G8B8));
    load();
}

PanoImageNode::~PanoImageNode ()
{
    clearTextures();
}

void PanoImageNode::connectDisplay()
{
    AreaNode::connectDisplay();
    
    setupTextures();
}

void PanoImageNode::disconnect(bool bKill)
{
    clearTextures();
    AreaNode::disconnect(bKill);
}

static ProfilingZoneID PanoRenderProfilingZone("PanoImageNode::render");

void PanoImageNode::render(const FRect& Rect)
{
    ScopeTimer Timer(PanoRenderProfilingZone);
    pushGLState();
    glproc::ActiveTexture(GL_TEXTURE0);

    gluLookAt(0, 0, 0,  // Eye
              0, 0, -1, // Center
              0, 1, 0); // Up.
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::render: gluLookAt()");

    glMatrixMode(GL_PROJECTION);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::render: glMatrixMode(GL_PROJECTION)");
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::render: glLoadIdentity()");

    calcProjection();
    gluPerspective(m_fovy*180/PI, m_aspect, 0.1, 2);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::render: gluPerspective()");
    glMatrixMode(GL_MODELVIEW);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::render: glMatrixMode(GL_MODELVIEW)");

    glDisable (GL_CLIP_PLANE0);
    glDisable (GL_CLIP_PLANE1);
    glDisable (GL_CLIP_PLANE2);
    glDisable (GL_CLIP_PLANE3);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::render: glDisable(GL_CLIP_PLANEx)");
    glm::vec2 Vpt = getSize();
    glViewport(0, 0, int(Vpt.x), int(Vpt.y));
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::render: glViewport()");
    glColor4d(1.0, 1.0, 1.0, getEffectiveOpacity());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::render: glColor4d()");
//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    float HorizOffset = m_Rotation+m_fovy*m_aspect/2;
//    glutWireSphere(1, 20, 16);
    for (unsigned int i=0; i<m_TileTextureIDs.size(); ++i) {
        unsigned int TexID = m_TileTextureIDs[i];
        glBindTexture(GL_TEXTURE_2D, TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "PanoImageNode::render: glBindTexture()");
        float StartAngle=i*m_SliceAngle-HorizOffset;
        float StartX = sin(StartAngle);
        float StartZ = -cos(StartAngle);
        float EndAngle;
        if (i<m_TileTextureIDs.size()-1) {
            EndAngle = (i+1)*m_SliceAngle-HorizOffset;
        } else {
            EndAngle = m_CylAngle-HorizOffset;
        }
        float EndX = sin(EndAngle);
        float EndZ = -cos(EndAngle);
        float TexPartUsed = float(m_pBmp->getSize().y)/m_TexHeight;
        glBegin(GL_QUADS);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(StartX, m_CylHeight, StartZ);
        glTexCoord2d(0.0, TexPartUsed);
        glVertex3d(StartX, -m_CylHeight, StartZ);
        glTexCoord2d(1.0, TexPartUsed);
        glVertex3d(EndX, -m_CylHeight, EndZ);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(EndX, m_CylHeight, EndZ);
        glEnd();
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "PanoImageNode::render: glEnd()");
    }

    popGLState();
}

float PanoImageNode::getScreenPosFromAngle(float Angle) const
{
    float HorizOffsetAngle = Angle-m_Rotation-m_fovy*m_aspect/2;
    float PixelDistFromCenter = m_FocalLength*tan(HorizOffsetAngle)/m_SensorWidth
            *getSize().x;
    return PixelDistFromCenter+getSize().x/2;
}

float PanoImageNode::getScreenPosFromPanoPos(int PanoPos) const
{
    float AnglePerPixel = m_CylAngle*1/float(m_pBmp->getSize().x);
    return getScreenPosFromAngle(AnglePerPixel*PanoPos);
}

const UTF8String& PanoImageNode::getHRef() const
{
    return m_href;
}

void PanoImageNode::setHRef(const UTF8String& href)
{
    m_href = href;
    load();
    if (getState() == NS_CANRENDER) {
        setupTextures();
    }
}

float PanoImageNode::getSensorWidth () const
{
    return m_SensorWidth;
}

void PanoImageNode::setSensorWidth (float sensorWidth)
{
    m_SensorWidth = sensorWidth;
}

float PanoImageNode::getSensorHeight () const
{
    return m_SensorHeight;
}

void PanoImageNode::setSensorHeight (float sensorHeight)
{
    m_SensorHeight = sensorHeight;
}

float PanoImageNode::getFocalLength () const
{
    return m_FocalLength;
}

void PanoImageNode::setFocalLength (float focalLength)
{
    m_FocalLength = focalLength;
}

float PanoImageNode::getRotation () const
{
    return m_Rotation;
}

void PanoImageNode::setRotation (float rotation)
{
    m_Rotation = rotation;
}

float PanoImageNode::getMaxRotation () const
{
    return m_MaxRotation;
}

void PanoImageNode::calcProjection()
{
    // Takes SensorWidth, SensorHeight and FocalLength and calculates
    // loads of derived values needed for projection.
    m_fovy = 2*atan((m_SensorHeight/2)/m_FocalLength);
    m_aspect = m_SensorWidth/m_SensorHeight;
    m_CylHeight = tan(m_fovy)/2;
    m_CylAngle = m_fovy*m_pBmp->getSize().x/m_pBmp->getSize().y;
    m_SliceAngle = m_CylAngle*TEX_WIDTH/float(m_pBmp->getSize().x);
    m_MaxRotation = m_CylAngle-m_fovy*m_aspect;
}

glm::vec2 PanoImageNode::getPreferredMediaSize()
{
    float SensorAspect = m_SensorWidth/m_SensorHeight;
    float Width = m_pBmp->getSize().y*SensorAspect;
    return glm::vec2(Width, m_pBmp->getSize().y);
}

void PanoImageNode::load()
{
    m_Filename = m_href;
    AVG_TRACE(Logger::MEMORY, "Loading " << m_Filename);
    if (m_Filename != "") {
        initFilename(m_Filename);
        try {
            
            m_pBmp = BitmapPtr(new Bitmap(m_Filename));
        } catch (Exception & ex) {
            AVG_TRACE(Logger::ERROR, ex.getStr());
        }
    }

    calcProjection();
    if (m_Rotation == -1) {
        m_Rotation = m_MaxRotation/2;
    }
}

void PanoImageNode::setupTextures()
{
    if (!m_TileTextureIDs.empty()) {
        clearTextures();
    }
    m_TexHeight = nextpow2(m_pBmp->getSize().y);
    int NumTextures = int(ceil(float(m_pBmp->getSize().x)/TEX_WIDTH));
    glproc::ActiveTexture(GL_TEXTURE0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::setupTextures: glproc::ActiveTexture(GL_TEXTURE0);");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::setupTextures: glPixelStorei(GL_UNPACK_ALIGNMENT)");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_pBmp->getSize().x);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::setupTextures: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glEnable(GL_TEXTURE_2D);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
            "PanoImageNode::setupTextures: glEnable(GL_TEXTURE_2D);");
    for (int i=0; i<NumTextures; i++) {
        BitmapPtr pRegion;
        if (i != NumTextures-1) {
            pRegion = BitmapPtr(new Bitmap(*m_pBmp,
                    IntRect(i*TEX_WIDTH, 0, (i+1)*TEX_WIDTH, m_pBmp->getSize().y)));
        } else {
            // The last column isn't necessarily as wide as the others.
            pRegion = BitmapPtr(new Bitmap(*m_pBmp,
                    IntRect(i*TEX_WIDTH, 0,
                           m_pBmp->getSize().x, m_pBmp->getSize().y)));
        }
        
        unsigned int TexID;
        glGenTextures(1, &TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "PanoImageNode::setupTextures: glGenTextures()");
        m_TileTextureIDs.push_back(TexID);
        glBindTexture(GL_TEXTURE_2D, TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "PanoImageNode::setupTextures: glBindTexture()");

        glTexParameteri(GL_TEXTURE_2D,
                GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,
                GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "PanoImageNode::setupTextures: glTexParameteri()");
        
        int DestMode;
        if (pRegion->getPixelFormat() == R8G8B8X8) {
            DestMode = GL_RGB;
        } else {
            DestMode = GL_RGBA;
        }
        glTexImage2D(GL_TEXTURE_2D, 0,
                DestMode, TEX_WIDTH, m_TexHeight, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, 0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "PanoImageNode::setupTextures: glTexImage2D()");
        unsigned char * pStartPos = pRegion->getPixels();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                pRegion->getSize().x, pRegion->getSize().y,
                GL_RGBA, GL_UNSIGNED_BYTE, pStartPos);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "PanoImageNode::setupTextures: glTexSubImage2D()");
   }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void PanoImageNode::clearTextures()
{
    for (unsigned int i=0; i<m_TileTextureIDs.size(); ++i) {
        unsigned int TexID = m_TileTextureIDs[i];
        glDeleteTextures(1, &TexID);
    }
    m_TileTextureIDs.clear();
}

}
