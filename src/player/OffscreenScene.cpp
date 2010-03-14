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

#include "OffscreenScene.h"

#include "SDLDisplayEngine.h"
#include "SceneNode.h"
#include "OGLTexture.h"

#include "../base/Exception.h"

#include <iostream>

using namespace boost;
using namespace std;

namespace avg {
    
OffscreenScene::OffscreenScene(Player * pPlayer, NodePtr pRootNode)
    : Scene(pPlayer, pRootNode)
{
    if (!pRootNode) {
        throw (Exception(AVG_ERR_XML_PARSE, 
                "Root node of a scene tree needs to be a <scene> node."));
    }
    if (pRootNode->getID() == "") {
        throw (Exception(AVG_ERR_XML_PARSE, 
                "Root node of a scene tree needs to have an id."));
    }
}

OffscreenScene::~OffscreenScene()
{
    glDeleteTextures(1, &m_TexID);
}

void OffscreenScene::initPlayback(DisplayEngine* pDisplayEngine, 
        AudioEngine* pAudioEngine, TestHelper* pTestHelper)
{
    Scene::initPlayback(pDisplayEngine, pAudioEngine, pTestHelper);
    glGenTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OffscreenScene::initPlayback: glGenTextures()");
    glBindTexture(GL_TEXTURE_2D, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OffscreenScene::initPlayback: glBindTexture()");
    IntPoint size = getSize();
    // Mipmaps needed for FBO support on nVidia cards (!?)
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, size.x);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OffscreenScene::initPlayback: glTexImage2D()");

    unsigned multiSampleSamples = dynamic_cast<SDLDisplayEngine*>(pDisplayEngine)
            ->getOGLOptions().m_MultiSampleSamples;
    m_pFBO = FBOPtr(new FBO(size, R8G8B8X8, m_TexID, multiSampleSamples));
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void OffscreenScene::render()
{
    m_pFBO->activate();
    getDisplayEngine()->render(getRootNode(), true);
    m_pFBO->deactivate();
    m_pFBO->copyToDestTexture();
    OGLTexturePtr pTex(new OGLTexture(getSize(), B8G8R8X8, 
            MaterialInfo(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true), 
            getDisplayEngine(), PBO));
    pTex->setTexID(m_TexID);
    BitmapPtr pBmp = pTex->readbackBmp();
    pBmp->save(getID()+".png");
}

std::string OffscreenScene::getID() const
{
    return getRootNode()->getID();
}

bool OffscreenScene::isRunning() const
{
    return (m_pFBO != FBOPtr());
}

unsigned OffscreenScene::getTexID() const
{
    return m_pFBO->getTexture();
}

}
