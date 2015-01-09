//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "ImageRegistry.h"

#include "../base/Exception.h"

using namespace std;

namespace avg {

ImageRegistry * ImageRegistry::s_pImageRegistry = 0;
    
ImageRegistry* ImageRegistry::get() 
{
    
    if (s_pImageRegistry == 0) {
        s_pImageRegistry = new ImageRegistry();
    }
    return s_pImageRegistry;
}


ImageRegistry::ImageRegistry()
{
}

ImageRegistry::~ImageRegistry()
{
}

ImagePtr ImageRegistry::getImage(const std::string& sFilename,
        Image::TextureCompression compression)
{
    ImageMap::iterator it = m_pImageMap.find(sFilename);
    ImagePtr pImg;
    if (it == m_pImageMap.end()) {
        pImg = ImagePtr(new Image(sFilename, compression));
        m_pImageMap[sFilename] = pImg;
    } else {
        pImg = it->second;
        pImg->incBmpRef(compression);
    }
    return pImg;
}

void ImageRegistry::deleteImage(const std::string& sFilename)
{
    ImageMap::iterator it = m_pImageMap.find(sFilename);
    AVG_ASSERT(it != m_pImageMap.end());
    m_pImageMap.erase(it);
}

int ImageRegistry::getNumImages() const
{
    return m_pImageMap.size();
}

}
