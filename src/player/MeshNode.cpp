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

#include "MeshNode.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include "TypeDefinition.h"
#include "VectorNode.h"

#include <cstdlib>
#include <string>
#include <iostream>

using namespace std;

namespace avg {

void MeshNode::registerType()
{
    vector<glm::vec2> vVert;
    vector<glm::vec2> vTex;
    vector<glm::ivec3> vTriangle;

    TypeDefinition def = TypeDefinition("mesh", "vectornode", 
            ExportedObject::buildObject<MeshNode>)
        .addArg(Arg<vector<glm::vec2> >("vertexcoords", vVert, false, 
                offsetof(MeshNode, m_VertexCoords)))
        .addArg(Arg<vector<glm::vec2> >("texcoords", vTex, false, 
                offsetof(MeshNode, m_TexCoords)))
        .addArg(Arg<vector<glm::ivec3> >("triangles", vTriangle, false, 
                offsetof(MeshNode, m_Triangles)))
        .addArg(Arg<bool>("backfacecull", false, false,
                offsetof(MeshNode, m_bBackfaceCull)))
        ;
    TypeRegistry::get()->registerType(def);
}

MeshNode::MeshNode(const ArgList& args)
    : VectorNode(args)
{
    args.setMembers(this);
    isValid(m_TexCoords);
}

MeshNode::~MeshNode()
{
}

void MeshNode::isValid(const vector<glm::vec2>& coords)
{
    if (coords.size() != m_VertexCoords.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                "Coordinates Out of Range"));
    }
}

const vector<glm::vec2>& MeshNode::getVertexCoords() const
{
    return m_VertexCoords;
}

void MeshNode::setVertexCoords(const vector<glm::vec2>& coords)
{
    isValid(coords);
    m_VertexCoords = coords;
    setDrawNeeded();
}

const vector<glm::vec2>& MeshNode::getTexCoords() const
{
    return m_TexCoords;
}

void MeshNode::setTexCoords(const vector<glm::vec2>& coords)
{
    isValid(coords);
    m_TexCoords = coords;
    setDrawNeeded();
}

const vector<glm::ivec3>& MeshNode::getTriangles() const
{
    return m_Triangles; 
}


void MeshNode::setTriangles(const vector<glm::ivec3>& triangles)
{
    for (unsigned int i = 0; i < triangles.size(); i++) {
        
        if (triangles[i].x < 0 || triangles[i].y < 0 || triangles[i].z < 0)
        {
            throw(Exception(AVG_ERR_OUT_OF_RANGE,
                "Triangle Index Out of Range < 0"));
        }
        
        if (static_cast<unsigned int>(triangles[i].x) > m_VertexCoords.size() || 
                static_cast<unsigned int>(triangles[i].y) > m_VertexCoords.size() ||
                static_cast<unsigned int>(triangles[i].z) > m_VertexCoords.size())
        {
            throw(Exception(AVG_ERR_OUT_OF_RANGE,
                "Triangle Index Out of Range > max triangles"));
        }
    }   
    m_Triangles = triangles;
    setDrawNeeded();
}

bool MeshNode::getBackfaceCull() const
{
    return m_bBackfaceCull;
}

void MeshNode::setBackfaceCull(const bool bBackfaceCull)
{
    m_bBackfaceCull = bBackfaceCull;
}

void MeshNode::calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    for (unsigned int i = 0; i < m_VertexCoords.size(); i++) {
        pVertexData->appendPos(m_VertexCoords[i],m_TexCoords[i], color);
    }

    for (unsigned int i = 0; i < m_Triangles.size(); i++) {
        pVertexData->appendTriIndexes(m_Triangles[i].x, m_Triangles[i].y, 
                m_Triangles[i].z);
    }
}

void MeshNode::render()
{
    if (m_bBackfaceCull) {
        glEnable(GL_CULL_FACE);
    }
    
    VectorNode::render();
    
    if (m_bBackfaceCull) {
        glDisable(GL_CULL_FACE);
    }
}

}
