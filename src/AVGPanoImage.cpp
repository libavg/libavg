//
// $Id$
// 

#include "AVGPanoImage.h"
#include "AVGSDLDisplayEngine.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGException.h"
#include "MathHelper.h"
#include "OGLHelper.h"

#include <paintlib/planydec.h>
#include <paintlib/Filter/plfilterresizebilinear.h>
#include <paintlib/Filter/plfilterfliprgb.h>
#include <paintlib/Filter/plfilterfill.h>
#include <paintlib/plpngenc.h>

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <iostream>
#include <sstream>
#include <math.h>

using namespace std;

NS_IMPL_ISUPPORTS1_CI(AVGPanoImage, IAVGNode);

const int TEX_WIDTH = 128;

AVGPanoImage * AVGPanoImage::create()
{
    return createNode<AVGPanoImage>("@c-base.org/avgpanoimage;1");
}       

AVGPanoImage::AVGPanoImage ()
{
    NS_INIT_ISUPPORTS();
}

AVGPanoImage::~AVGPanoImage ()
{
}

NS_IMETHODIMP 
AVGPanoImage::GetType(PRInt32 *_retval)
{
    *_retval = NT_PANOIMAGE;
    return NS_OK;
}

void AVGPanoImage::init (const std::string& id, const std::string& filename, 
        double SensorWidth, double SensorHeight, double FocalLength, 
        IAVGDisplayEngine * pEngine, AVGContainer * pParent, 
        AVGPlayer * pPlayer)
{
    AVGNode::init(id, pEngine, pParent, pPlayer);
    m_pEngine = dynamic_cast<AVGSDLDisplayEngine*>(pEngine);

    if (!m_pEngine) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                "Panorama images are only allowed when "
                "the display engine is OpenGL. Aborting.");
        exit(-1);
    }

    m_Filename = filename;
    AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "Loading " << m_Filename);

    PLAnyPicDecoder decoder;
    PLAnyBmp TempBmp;
    decoder.MakeBmpFromFile(m_Filename.c_str(), &TempBmp, 32);
    if (!pEngine->hasRGBOrdering()) {
        TempBmp.ApplyFilter(PLFilterFlipRGB());
    }
    m_Bmp.CreateCopy(TempBmp, 32);

    m_SensorWidth = SensorWidth;
    m_SensorHeight = SensorHeight;
    m_FocalLength = FocalLength;
    
    setupTextures();
}

void AVGPanoImage::render (const AVGDRect& Rect)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glPushAttrib()");
    glPushClientAttrib(GL_ALL_CLIENT_ATTRIB_BITS);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glPushClientAttrib()");
    glPushMatrix();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glPushMatrix()");
    glDisable(AVGOGLSurface::getTextureMode());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glDisable(Old texture mode);");
    glEnable(GL_TEXTURE_2D);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glEnable(GL_TEXTURE_2D);");

    gluLookAt(0, 0, 0,  // Eye
              0, 0, -1, // Center
              0, 1, 0); // Up.
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: gluLookAt()");

    glMatrixMode(GL_PROJECTION);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glMatrixMode(GL_PROJECTION)");
    glPushMatrix();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glPushMatrix()");
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glLoadIdentity()");

    double fovy = 2*atan((m_SensorHeight/2)/m_FocalLength)*180.0/PI;
    double aspect = m_SensorWidth/m_SensorHeight;
//    cerr << "fovy: " << fovy << ", aspect: " << aspect << endl;
    gluPerspective(fovy, aspect, 0.1, 2);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: gluPerspective()");
    glMatrixMode(GL_MODELVIEW);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glMatrixMode(GL_MODELVIEW)");

    glDisable (GL_CLIP_PLANE0);
    glDisable (GL_CLIP_PLANE1);
    glDisable (GL_CLIP_PLANE2);
    glDisable (GL_CLIP_PLANE3);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glDisable(GL_CLIP_PLANEx)");
    AVGDRect Vpt = getAbsViewport();
    glViewport(int(Vpt.tl.x), int(Vpt.tl.y), 
            int(Vpt.Width()), int(Vpt.Height()));
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glViewport()");
    glColor4f(1.0f, 1.0f, 1.0f, getEffectiveOpacity());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glColor4f()");

//    glutWireSphere(1, 20, 16);
    double CylHeight = sin(fovy*PI/180.0)/2;
    double CylAngle = fovy*PI/180.0*m_Bmp.GetWidth()/m_Bmp.GetHeight();
    double SliceAngle = CylAngle*TEX_WIDTH/double(m_Bmp.GetWidth());
//    cerr << "CylHeight: " << CylHeight << ", CylAngle: " << CylAngle 
//            << ", SliceAngle: " << SliceAngle << endl;
    for (int i=0; i<m_TileTextureIDs.size(); ++i) {
        unsigned int TexID = m_TileTextureIDs[i];
        glBindTexture(GL_TEXTURE_2D, TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "AVGPanoImage::render: glBindTexture()");
        double StartAngle=i*SliceAngle-CylAngle/2;
        double StartX = sin(StartAngle);
        double StartZ = -cos(StartAngle);
        double EndAngle;
        if (i<m_TileTextureIDs.size()-1) {
            EndAngle = (i+1)*SliceAngle-CylAngle/2;
        } else {
            EndAngle = CylAngle-CylAngle/2;
        }
        double EndX = sin(EndAngle);
        double EndZ = -cos(EndAngle);
        
        glBegin(GL_QUADS);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(StartX, CylHeight, StartZ);
        glTexCoord2d(0.0, 1.0);
        glVertex3d(StartX, -CylHeight, StartZ);
        glTexCoord2d(1.0, 1.0);
        glVertex3d(EndX, -CylHeight, EndZ);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(EndX, CylHeight, EndZ);
        glEnd();
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "AVGPanoImage::render: glEnd()");
    }
    
    // Restore previous GL state.
    glViewport(0, 0, m_pEngine->getWidth(), m_pEngine->getHeight());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glViewport() restore");
    glEnable(AVGOGLSurface::getTextureMode());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glEnable(Old texture mode);");
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopClientAttrib();
    glPopAttrib();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::render: glPopAttrib()");
}

bool AVGPanoImage::obscures (const AVGDRect& Rect, int z) 
{
    return (getEffectiveOpacity() > 0.999 && !m_Bmp.HasAlpha() 
            && getZ() > z && getVisibleRect().Contains(Rect));
}

string AVGPanoImage::getTypeStr ()
{
    return "AVGPanoImage";
}

AVGDPoint AVGPanoImage::getPreferredMediaSize()
{
    double SensorAspect = m_SensorWidth/m_SensorHeight;
    double Width = m_Bmp.GetHeight()*SensorAspect;
    return AVGDPoint(Width, m_Bmp.GetHeight());
}

void AVGPanoImage::setupTextures()
{
    int TexHeight = nextpow2(m_Bmp.GetHeight());
    int NumTextures = int(ceil(double(m_Bmp.GetWidth())/TEX_WIDTH));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::setupTextures: glPixelStorei(GL_UNPACK_ALIGNMENT)");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_Bmp.GetWidth());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::setupTextures: glPixelStorei(GL_UNPACK_ROW_LENGTH)");
    glEnable(GL_TEXTURE_2D);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGPanoImage::setupTextures: glEnable(GL_TEXTURE_2D);");
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
                "AVGPanoImage::setupTextures: glGenTextures()");
        m_TileTextureIDs.push_back(TexID);
        glBindTexture(GL_TEXTURE_2D, TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "AVGPanoImage::setupTextures: glBindTexture()");

        glTexParameteri(GL_TEXTURE_2D, 
                GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, 
                GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "AVGPanoImage::setupTextures: glTexParameteri()");

        glTexImage2D(GL_TEXTURE_2D, 0,
                GL_RGBA, TEX_WIDTH, TexHeight, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, 0); 
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "AVGPanoImage::setupTextures: glTexImage2D()");
        PLBYTE * pStartPos = Region.GetLineArray()[0];
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                Region.GetWidth(), Region.GetHeight(),
                GL_RGBA, GL_UNSIGNED_BYTE, pStartPos);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "AVGPanoImage::setupTextures: glTexSubImage2D()");

   }
}

