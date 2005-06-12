//
// $Id$
// 

#include "PanoImage.h"
#include "PanoImageFactory.h"
#include "SDLDisplayEngine.h"
#include "MathHelper.h"
#include "OGLHelper.h"

#include "../base/Logger.h"
#include "../base/ProfilingZone.h"
#include "../base/ScopeTimer.h"
#include "../base/Exception.h"

#include <paintlib/planydec.h>
#include <paintlib/Filter/plfilterresizebilinear.h>
#include <paintlib/Filter/plfilterfliprgb.h>
#include <paintlib/Filter/plfilterfill.h>
#include <paintlib/Filter/plfiltercolorize.h>
#include <paintlib/plpngenc.h>

#include "GL/gl.h"
#include "GL/glu.h"

#include <iostream>
#include <sstream>
#include <math.h>

using namespace std;

const int TEX_WIDTH = 64;

namespace avg {

PanoImage::PanoImage ()
    : m_Filename(""),
      m_SensorWidth(0),
      m_SensorHeight(0),
      m_FocalLength(0),
      m_Rotation(-1),
      m_Hue(-1),
      m_Saturation(-1)
{
}

PanoImage::~PanoImage ()
{
}

void PanoImage::init (IDisplayEngine * pEngine, 
        Container * pParent, Player * pPlayer)
{
    Node::init(pEngine, pParent, pPlayer);
    initFilename(pPlayer, m_Filename);
    m_pEngine = dynamic_cast<SDLDisplayEngine*>(pEngine);
    if (!m_pEngine) {
        AVG_TRACE(Logger::ERROR, 
                "Panorama images are only allowed when "
                "the display engine is OpenGL. Aborting.");
        // TODO: Disable image.
        exit(-1);
    }

    AVG_TRACE(Logger::PROFILE, "Loading " << m_Filename);

    PLAnyPicDecoder decoder;
    decoder.MakeBmpFromFile(m_Filename.c_str(), &m_Bmp, 
            PLPixelFormat::A8R8G8B8);
       
    if (m_Saturation != -1) {
        m_Bmp.ApplyFilter(PLFilterColorize(m_Hue, m_Saturation));
    }
    if (pEngine->hasRGBOrdering()) {
        m_Bmp.ApplyFilter(PLFilterFlipRGB());
    }
 
    calcProjection();
    if (m_Rotation == -1) {
        m_Rotation = m_MaxRotation/2;
    }
    setupTextures();
}

static ProfilingZone PanoRenderProfilingZone("  PanoImage::render");
//static ProfilingZone PanoRenderQuadsProfilingZone("PanoImage::render quads");

void PanoImage::render(const DRect& Rect)
{
    ScopeTimer Timer(PanoRenderProfilingZone);
    glPushMatrix();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: glPushMatrix()");
    if (OGLSurface::getTextureMode() != GL_TEXTURE_2D) {
        glDisable(OGLSurface::getTextureMode());
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::render: glDisable(Old texture mode);");
        glEnable(GL_TEXTURE_2D);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::render: glEnable(GL_TEXTURE_2D);");
    }

    gluLookAt(0, 0, 0,  // Eye
              0, 0, -1, // Center
              0, 1, 0); // Up.
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: gluLookAt()");

    glMatrixMode(GL_PROJECTION);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: glMatrixMode(GL_PROJECTION)");
    glPushMatrix();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: glPushMatrix()");
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: glLoadIdentity()");

    calcProjection();
    gluPerspective(m_fovy*180/PI, m_aspect, 0.1, 2);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: gluPerspective()");
    glMatrixMode(GL_MODELVIEW);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: glMatrixMode(GL_MODELVIEW)");

    glDisable (GL_CLIP_PLANE0);
    glDisable (GL_CLIP_PLANE1);
    glDisable (GL_CLIP_PLANE2);
    glDisable (GL_CLIP_PLANE3);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: glDisable(GL_CLIP_PLANEx)");
    DRect Vpt = getAbsViewport();
    glViewport(int(Vpt.tl.x), int(Vpt.tl.y), 
            int(Vpt.Width()), int(Vpt.Height()));
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: glViewport()");
    glColor4f(1.0f, 1.0f, 1.0f, getEffectiveOpacity());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: glColor4f()");
//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    double HorizOffset = m_Rotation+m_fovy*m_aspect/2;
//    glutWireSphere(1, 20, 16);
    for (unsigned int i=0; i<m_TileTextureIDs.size(); ++i) {
//        ScopeTimer ScopeTimer(PanoRenderQuadsProfilingZone);
        unsigned int TexID = m_TileTextureIDs[i];
        glBindTexture(GL_TEXTURE_2D, TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::render: glBindTexture()");
        double StartAngle=i*m_SliceAngle-HorizOffset;
        double StartX = sin(StartAngle);
        double StartZ = -cos(StartAngle);
        double EndAngle;
        if (i<m_TileTextureIDs.size()-1) {
            EndAngle = (i+1)*m_SliceAngle-HorizOffset;
        } else {
            EndAngle = m_CylAngle-HorizOffset;
        }
        double EndX = sin(EndAngle);
        double EndZ = -cos(EndAngle);
        
        glBegin(GL_QUADS);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(StartX, m_CylHeight, StartZ);
        glTexCoord2d(0.0, 1.0);
        glVertex3d(StartX, -m_CylHeight, StartZ);
        glTexCoord2d(1.0, 1.0);
        glVertex3d(EndX, -m_CylHeight, EndZ);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(EndX, m_CylHeight, EndZ);
        glEnd();
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::render: glEnd()");
    }
    
    // Restore previous GL state.
    glViewport(0, 0, m_pEngine->getWidth(), m_pEngine->getHeight());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::render: glViewport() restore");
    if (OGLSurface::getTextureMode() != GL_TEXTURE_2D) {
        glDisable(GL_TEXTURE_2D);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::render: glDisable(GL_TEXTURE_2D);");
        glEnable(OGLSurface::getTextureMode());
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::render: glEnable(Old texture mode);");
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

bool PanoImage::obscures (const DRect& Rect, int z) 
{
    return (isActive() && getEffectiveOpacity() > 0.999 && !m_Bmp.HasAlpha() 
            && getZ() > z && getVisibleRect().Contains(Rect));
}

string PanoImage::getTypeStr ()
{
    return "PanoImage";
}

JSFactoryBase* PanoImage::getFactory()
{
    return PanoImageFactory::getInstance();
}

void PanoImage::calcProjection()
{
    // Takes SensorWidth, SensorHeight and FocalLength and calculates
    // loads of derived values needed for projection.
    m_fovy = 2*atan((m_SensorHeight/2)/m_FocalLength);
    m_aspect = m_SensorWidth/m_SensorHeight;
    m_CylHeight = tan(m_fovy)/2;
    m_CylAngle = m_fovy*m_Bmp.GetWidth()/m_Bmp.GetHeight();
    m_SliceAngle = m_CylAngle*TEX_WIDTH/double(m_Bmp.GetWidth());
    m_MaxRotation = m_CylAngle-m_fovy*m_aspect;
}

DPoint PanoImage::getPreferredMediaSize()
{
    double SensorAspect = m_SensorWidth/m_SensorHeight;
    double Width = m_Bmp.GetHeight()*SensorAspect;
    return DPoint(Width, m_Bmp.GetHeight());
}

void PanoImage::setupTextures()
{
    int TexHeight = nextpow2(m_Bmp.GetHeight());
    int NumTextures = int(ceil(double(m_Bmp.GetWidth())/TEX_WIDTH));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::setupTextures: glPixelStorei(GL_UNPACK_ALIGNMENT)");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_Bmp.GetWidth());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::setupTextures: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glEnable(GL_TEXTURE_2D);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "PanoImage::setupTextures: glEnable(GL_TEXTURE_2D);");
    for (int i=0; i<NumTextures; i++) {
        PLSubBmp Region;
        if (i != NumTextures-1) {
            Region.Create(m_Bmp, 
                    PLRect(i*TEX_WIDTH, 0, (i+1)*TEX_WIDTH, m_Bmp.GetHeight()));
        } else {
            // The last column isn't necessarily as wide as the others.
            Region.Create (m_Bmp,
                    PLRect(i*TEX_WIDTH, 0, 
                           m_Bmp.GetWidth(), m_Bmp.GetHeight()));
        }
        
        unsigned int TexID;
        glGenTextures(1, &TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::setupTextures: glGenTextures()");
        m_TileTextureIDs.push_back(TexID);
        glBindTexture(GL_TEXTURE_2D, TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "PanoImage::setupTextures: glBindTexture()");

        glTexParameteri(GL_TEXTURE_2D, 
                GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, 
                GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::setupTextures: glTexParameteri()");

        glTexImage2D(GL_TEXTURE_2D, 0,
                GL_RGBA, TEX_WIDTH, TexHeight, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, 0); 
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::setupTextures: glTexImage2D()");
        PLBYTE * pStartPos = Region.GetLineArray()[0];
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                Region.GetWidth(), Region.GetHeight(),
                GL_RGBA, GL_UNSIGNED_BYTE, pStartPos);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "PanoImage::setupTextures: glTexSubImage2D()");

   }
}

}
