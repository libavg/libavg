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
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OffscreenScene::initPlayback: glBindTexture()");
    IntPoint size = getSize();
    glPixelStorei(GL_UNPACK_ROW_LENGTH, size.x);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, size.x, size.y, 0,
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
//    BitmapPtr pBmp = m_pFBO->getImage(0);
//    pBmp->save("foo.png");
}

std::string OffscreenScene::getID() const
{
    return getRootNode()->getID();
}

}
