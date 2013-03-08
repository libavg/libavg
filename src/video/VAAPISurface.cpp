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
#include "VAAPISurface.h"

#include "../base/Exception.h"

#include <iostream>

using namespace std;
    
namespace avg {

VAAPISurface::VAAPISurface(VASurfaceID surfaceID, VAAPIDecoder* pDecoder)
    : m_SurfaceID(surfaceID),
      m_bUsed(false)
{
}

VASurfaceID VAAPISurface::getSurfaceID() const
{
    return m_SurfaceID;
}

void VAAPISurface::setUsed(bool bUsed)
{
    AVG_ASSERT(bUsed != m_bUsed);
    m_bUsed = bUsed;
}

void VAAPISurface::getYUVBmps(BitmapPtr pBmpY, BitmapPtr pBmpU, BitmapPtr pBmpV) const
{
/*
    VAAPIDecoder* pVaapiDecoder = pVaapiSurface->m_pDecoder;
    VASurfaceID surface_id = pVaapiSurface->m_SurfaceID;
    VAImage image = pVaapiSurface->m_Image;

    if(vaSyncSurface(getVAAPIDisplay(), pVaapiSurface->m_SurfaceID)) {
        AVG_ASSERT_MSG(false, "vaSyncSurface returned error");
        return;
    }
    if (pVaapiSurface->m_bBound == 0) {
        if(vaGetImage(getVAAPIDisplay(), surface_id, 0, 0, pVaapiDecoder->getSize().x,
                    pVaapiDecoder->getSize().y, image.image_id) ) {
            AVG_ASSERT_MSG(false, "vaGetImage returned error");
            return;
        }
    }

    void *p_base;
    if(vaMapBuffer(getVAAPIDisplay(), image.buf, &p_base)) {
        AVG_ASSERT_MSG(false, "vaMapBuffer returned error");
        return;
    }

    uint8_t *dest[3] = {
        pBmpY->getPixels(),
        pBmpV->getPixels(),
        pBmpU->getPixels()
    };
    uint32_t pitches[3] = {
        pBmpY->getStride(),
        pBmpV->getStride(),
        pBmpU->getStride()
    };

    const uint32_t i_fourcc = pVaapiDecoder->getImage().format.fourcc;
    //AVG_TRACE(Logger::category::PLAYER, Logger::severity::INFO, "getPlanesFromVAAPI: " << i_fourcc);

    if(i_fourcc == VA_FOURCC('Y','V','1','2') ||
            i_fourcc == VA_FOURCC('I','4','2','0')) {
        bool b_swap_uv = i_fourcc == VA_FOURCC('I','4','2','0');
        uint8_t *pp_plane[3];
        size_t  pi_pitch[3];

        for( int i = 0; i < 3; i++ ) {
            const int i_src = (b_swap_uv && i != 0) ? (3-i) : i;
            pp_plane[i] = (uint8_t*)p_base + image.offsets[i_src];
            pi_pitch[i] = image.pitches[i_src];
        }

        copyPlane(dest[0], pitches[0], pp_plane[0], pi_pitch[0],
                pVaapiDecoder->getSize().x, pVaapiDecoder->getSize().y);
        copyPlane(dest[1], pitches[1], pp_plane[1], pi_pitch[1],
                pVaapiDecoder->getSize().x/2, pVaapiDecoder->getSize().y/2);
        copyPlane(dest[2], pitches[2], pp_plane[2], pi_pitch[2],
                pVaapiDecoder->getSize().x/2, pVaapiDecoder->getSize().y/2);
    }
    else
    {
        assert( i_fourcc == VA_FOURCC('N','V','1','2') );
        uint8_t *pp_plane[2];
        size_t  pi_pitch[2];

        for(int i = 0; i < 2; i++) {
            pp_plane[i] = (uint8_t*)p_base + pVaapiDecoder->getImage().offsets[i];
            pi_pitch[i] = pVaapiDecoder->getImage().pitches[i];
        }

        copyPlane(dest[0], pitches[0], pp_plane[0], pi_pitch[0],
                pVaapiDecoder->getSize().x, pVaapiDecoder->getSize().y);
        splitPlanes(dest[2], pitches[2], dest[1], pitches[1],
                pp_plane[1], pi_pitch[1],
                pVaapiDecoder->getSize().x/2, pVaapiDecoder->getSize().y/2);
    }

    if(vaUnmapBuffer(getVAAPIDisplay(), image.buf)) {
        AVG_ASSERT_MSG(false, "vaUnmapBuffer returned error");
        return;
    }

    //AVG_TRACE(Logger::category::PLAYER, Logger::severity::INFO, "getPlanesFromVaapi finished.");
    unlockVAAPISurface(pVaapiSurface);
   */ 
}

void VAAPISurface::getRGBBmp(BitmapPtr pBmp) const
{
}

}

