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

#ifndef _VertexArray_H_
#define _VertexArray_H_

#include "../api.h"
#include "../base/Point.h"
#include "../graphics/Pixel32.h"
#include "../graphics/OGLHelper.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread/tss.hpp>

namespace avg {

struct T2V3C4Vertex {
    GLfloat m_Tex[2];
    Pixel32 m_Color;
    GLfloat m_Pos[3];
};

class AVG_API VertexArray {
public:
    VertexArray(int reserveVerts = 0, int reserveIndexes = 0);
    virtual ~VertexArray();

    virtual void appendPos(const DPoint& pos, 
            const DPoint& texPos, const Pixel32& color = Pixel32(0,0,0,0));
    void appendTriIndexes(int v0, int v1, int v2);
    void appendQuadIndexes(int v0, int v1, int v2, int v3);
    void addLineData(Pixel32 color, const DPoint& p1, const DPoint& p2, double width,
            double tc1=0, double tc2=1);
    void reset();

    void update();
    void draw();

    int getCurVert() const;
    int getCurIndex() const;
    void dump() const;

    void initBufferCache();
    static void deleteBufferCache();

private:
    void grow();

    int m_NumVerts;
    int m_NumIndexes;
    int m_ReserveVerts;
    int m_ReserveIndexes;
    T2V3C4Vertex * m_pVertexData;
    unsigned int * m_pIndexData;

    bool m_bDataChanged;

    unsigned int m_GLVertexBufferID;
    unsigned int m_GLIndexBufferID;

    // TODO: This assumes one GL context per thread.
    static boost::thread_specific_ptr<std::vector<unsigned int> > s_pGLVertexBufferIDs;
    static boost::thread_specific_ptr<std::vector<unsigned int> > s_pGLIndexBufferIDs;
};

typedef boost::shared_ptr<VertexArray> VertexArrayPtr;

}

#endif
