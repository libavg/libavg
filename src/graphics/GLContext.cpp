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

#include "GLContext.h"

#include "ShaderRegistry.h"
#include "StandardShader.h"
#include "GLContextManager.h"

#include "../base/Backtrace.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/MathHelper.h"

#include <iostream>
#include <stdio.h>


namespace avg {

using namespace std;
using namespace boost;

thread_specific_ptr<GLContext*> GLContext::s_pCurrentContext;
bool GLContext::s_bErrorCheckEnabled = false;
bool GLContext::s_bErrorLogEnabled = true;


GLContext::GLContext(const IntPoint& windowSize)
    : m_MaxTexSize(0),
      m_bCheckedGPUMemInfoExtension(false),
      m_bCheckedMemoryMode(false),
      m_BlendColor(0.f, 0.f, 0.f, 0.f),
      m_BlendMode(BLEND_ADD),
      m_MajorGLVersion(-1)
{
    if (s_pCurrentContext.get() == 0) {
        s_pCurrentContext.reset(new (GLContext*));
    }
    GLContextManager::get()->registerContext(this);
}

GLContext::~GLContext()
{
    GLContextManager::get()->unregisterContext(this);
}

void GLContext::init(const GLConfig& glConfig, bool bOwnsContext)
{
    m_GLConfig = glConfig;
    m_bOwnsContext = bOwnsContext;
    activate();
    glproc::init();
    if (m_GLConfig.m_bGLES) {
        m_MajorGLVersion = 2;
        m_MinorGLVersion = 0;
    } else {
        const char* pVersion = (const char*)glGetString(GL_VERSION);
        sscanf(pVersion, "%d.%d", &m_MajorGLVersion, &m_MinorGLVersion);
    }

    if (m_GLConfig.m_bUseDebugContext) {
        if (isDebugContextSupported()) {
            glproc::DebugMessageCallback(GLContext::debugLogCallback, 0);
        } else {
            m_GLConfig.m_bUseDebugContext = false;
        }
    }
#ifndef AVG_ENABLE_EGL
    if (m_GLConfig.m_MultiSampleSamples > 1) {
        glEnable(GL_MULTISAMPLE);
        checkError("init: glEnable(GL_MULTISAMPLE)");
    }
#endif
    m_pShaderRegistry = ShaderRegistryPtr(new ShaderRegistry());
    if (useGPUYUVConversion()) {
        m_pShaderRegistry->setPreprocessorDefine("ENABLE_YUV_CONVERSION", "");
    }
    setBlendMode(BLEND_BLEND, false);
    if (!m_GLConfig.m_bUsePOTTextures) {
        m_GLConfig.m_bUsePOTTextures = 
                !queryOGLExtension("GL_ARB_texture_non_power_of_two") && !isGLES();
    }
    if (m_GLConfig.m_ShaderUsage == GLConfig::AUTO) {
        if (isGLES()) {
            m_GLConfig.m_ShaderUsage = GLConfig::MINIMAL;
        } else {
            m_GLConfig.m_ShaderUsage = GLConfig::FULL;
        }
#ifdef __APPLE__
        if (GLContext::isVendor("Intel")) {
            // Bug #434: Some shaders cause hard lockups on Mac Book Air.
            m_GLConfig.m_ShaderUsage = GLConfig::MINIMAL;
        }
#endif
    }
    for (int i=0; i<16; ++i) {
        m_BoundTextures[i] = 0xFFFFFFFF;
    }
    if (!m_GLConfig.m_bGLES && !queryOGLExtension("GL_ARB_vertex_buffer_object")) {
        throw Exception(AVG_ERR_UNSUPPORTED,
           "Graphics driver lacks vertex buffer support, unable to initialize graphics.");
    }
    glEnable(GL_BLEND);
    checkError("init: glEnable(GL_BLEND)");
    glDisable(GL_DEPTH_TEST);
    checkError("init: glDisable(GL_DEPTH_TEST)");
    glEnable(GL_STENCIL_TEST);
    checkError("init: glEnable(GL_STENCIL_TEST)");
}

void GLContext::deleteObjects()
{
    m_pStandardShader = StandardShaderPtr();
    for (unsigned i=0; i<m_FBOIDs.size(); ++i) {
        glproc::DeleteFramebuffers(1, &(m_FBOIDs[i]));
    }
    m_FBOIDs.clear();
    if (*s_pCurrentContext == this) {
        *s_pCurrentContext = 0;
    }
}

void GLContext::getVersion(int& major, int& minor) const
{
    major = m_MajorGLVersion;
    minor = m_MinorGLVersion;
}

bool GLContext::ownsContext() const
{
    return m_bOwnsContext;
}

void GLContext::setCurrent()
{
    *s_pCurrentContext = this;
}

ShaderRegistryPtr GLContext::getShaderRegistry() const
{
    return m_pShaderRegistry;
}

StandardShaderPtr GLContext::getStandardShader()
{
    if (m_pStandardShader == StandardShaderPtr()) {
        m_pStandardShader = StandardShaderPtr(new StandardShader());
    }
    return m_pStandardShader;
}

bool GLContext::useGPUYUVConversion() const
{
    return (m_MajorGLVersion > 1) || isGLES();
}

GLConfig::ShaderUsage GLContext::getShaderUsage() const
{
    return m_GLConfig.m_ShaderUsage;
}

GLBufferCache& GLContext::getPBOCache()
{
    return m_PBOCache;
}

unsigned GLContext::genFBO()
{
    unsigned fboID;
    if (m_FBOIDs.empty()) {
        glproc::GenFramebuffers(1, &fboID);
    } else {
        fboID = m_FBOIDs.back();
        m_FBOIDs.pop_back();
    }
    return fboID;
}

void GLContext::returnFBOToCache(unsigned fboID) 
{
    m_FBOIDs.push_back(fboID);
}

void GLContext::setBlendColor(const glm::vec4& color)
{
    if (m_BlendColor != color) {
        glproc::BlendColor(color[0], color[1], color[2], color[3]);
        m_BlendColor = color;
    }
}

void GLContext::setBlendMode(BlendMode mode, bool bPremultipliedAlpha)
{
    AVG_ASSERT(isBlendModeSupported(mode));
    GLenum srcFunc;
    if (bPremultipliedAlpha) {
        srcFunc = GL_CONSTANT_ALPHA;
    } else {
        srcFunc = GL_SRC_ALPHA;
    }
    if (mode != m_BlendMode || m_bPremultipliedAlpha != bPremultipliedAlpha) {
        switch (mode) {
            case BLEND_BLEND:
                glproc::BlendEquation(GL_FUNC_ADD);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkError("setBlendMode: blend");
                break;
            case BLEND_ADD:
                glproc::BlendEquation(GL_FUNC_ADD);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE, GL_ONE, GL_ONE);
                checkError("setBlendMode: add");
                break;
            case BLEND_MIN:
                glproc::BlendEquation(GL_MIN_EXT);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkError("setBlendMode: min");
                break;
            case BLEND_MAX:
                glproc::BlendEquation(GL_MAX_EXT);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkError("setBlendMode: max");
                break;
            case BLEND_COPY:
                glproc::BlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ZERO);
                checkError("setBlendMode: copy");
                break;
            default:
                AVG_ASSERT(false);
        }

        m_BlendMode = mode;
        m_bPremultipliedAlpha = bPremultipliedAlpha;
    }
}

bool GLContext::isBlendModeSupported(BlendMode mode) const
{
    if (isGLES() && (mode == BLEND_MIN || mode == BLEND_MAX)) {
        return queryOGLExtension("GL_EXT_blend_minmax");
    } else {
        return true;
    }
}

void GLContext::bindTexture(unsigned unit, unsigned texID)
{
    if (m_BoundTextures[unit-GL_TEXTURE0] != texID) {
        glproc::ActiveTexture(unit);
        checkError("GLContext::bindTexture ActiveTexture()");
        glBindTexture(GL_TEXTURE_2D, texID);
        checkError("GLContext::bindTexture BindTexture()");
        m_BoundTextures[unit-GL_TEXTURE0] = texID;
    }
}

const GLConfig& GLContext::getConfig()
{
    return m_GLConfig;
}

void GLContext::logConfig() 
{
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO, "OpenGL configuration: ");
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "  OpenGL version: " << glGetString(GL_VERSION));
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "  OpenGL vendor: " << glGetString(GL_VENDOR));
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "  OpenGL renderer: " << glGetString(GL_RENDERER));
    m_GLConfig.log();
    switch (getMemoryMode()) {
        case MM_PBO:
            AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                    "  Using pixel buffer objects");
            break;
        case MM_OGL:
            AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                    "  Not using GL memory extensions");
            break;
    }
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "  Max. texture size: " << getMaxTexSize());
    string s;
    if (useGPUYUVConversion()) {
        s = "yes";
    } else {
        s = "no";
    }
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            string("  GPU-based YUV-RGB conversion: ")+s+".");
    try {
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                "  Dedicated video memory: " << getVideoMemInstalled()/(1024*1024)
                << " MB");
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                "  Video memory used at start: " << getVideoMemUsed()/(1024*1024)
                << " MB");
    } catch (Exception) {
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::ERROR,
                "  Dedicated video memory: Unknown");
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::ERROR,
                "  Video memory used at start: Unknown");
    }
}

size_t GLContext::getVideoMemInstalled()
{
    checkGPUMemInfoSupport();
    int kbMemInstalled;
    glGetIntegerv(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &kbMemInstalled);
    return (size_t)kbMemInstalled*1024;
}

size_t GLContext::getVideoMemUsed()
{
    checkGPUMemInfoSupport();
    int kbMemAvailable;
    glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &kbMemAvailable);
    return getVideoMemInstalled()-(size_t)kbMemAvailable*1024;
}

bool GLContext::usePOTTextures()
{
    return m_GLConfig.m_bUsePOTTextures;
}

bool GLContext::arePBOsSupported()
{
    if (isGLES()) {
        return false;
    } else {
        return (queryOGLExtension("GL_ARB_pixel_buffer_object") || 
                 queryOGLExtension("GL_EXT_pixel_buffer_object"));
    }
}

OGLMemoryMode GLContext::getMemoryMode()
{
    if (!m_bCheckedMemoryMode) {
        if (arePBOsSupported() && m_GLConfig.m_bUsePixelBuffers) {
            m_MemoryMode = MM_PBO;
        } else {
            m_MemoryMode = MM_OGL;
        }
        m_bCheckedMemoryMode = true;
    }
    return m_MemoryMode;
}
    
bool GLContext::isGLES() const
{
    return m_GLConfig.m_bGLES;
}

bool GLContext::isVendor(const string& sWantedVendor) const
{
    string sVendor((const char *)glGetString(GL_VENDOR));
    return (sVendor.find(sWantedVendor) != string::npos);
}

bool GLContext::useDepthBuffer() const
{
    return !isGLES();
}

int GLContext::getMaxTexSize() 
{
    if (m_MaxTexSize == 0) {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_MaxTexSize);
    }
    return m_MaxTexSize;
}


void GLContext::swapBuffers()
{
    AVG_ASSERT(false);
}

void GLContext::enableErrorChecks(bool bEnable)
{
    s_bErrorCheckEnabled = bEnable;
}
    
void GLContext::checkError(const char* pszWhere) 
{
    if (s_bErrorCheckEnabled) {
        mandatoryCheckError(pszWhere);
    }
}

void GLContext::mandatoryCheckError(const char* pszWhere) 
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        stringstream s;
#ifndef AVG_ENABLE_EGL
        s << "OpenGL error in " << pszWhere <<": " << gluErrorString(err) 
            << " (#" << err << ") ";
#else
        s << "OpenGL error in " << pszWhere <<": (#" << err << ") ";
#endif
        AVG_LOG_ERROR(s.str());
        if (err != GL_INVALID_OPERATION) {
            checkError("  --");
        }
        AVG_ASSERT(false);
    }
}
    
void GLContext::ensureFullShaders(const string& sContext) const
{
    if (getShaderUsage() != GLConfig::FULL) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                sContext + " not supported if ShaderUsage==MINIMAL");
    }

}

GLContext::BlendMode GLContext::stringToBlendMode(const string& s)
{
    if (s == "blend") {
        return GLContext::BLEND_BLEND;
    } else if (s == "add") {
        return GLContext::BLEND_ADD;
    } else if (s == "min") {
        return GLContext::BLEND_MIN;
    } else if (s == "max") {
        return GLContext::BLEND_MAX;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, "Blend mode "+s+" not supported."));
    }
}

GLContext* GLContext::getCurrent()
{
    return *s_pCurrentContext;
}

int GLContext::nextMultiSampleValue(int curSamples)
{
    switch (curSamples) {
        case 1:
            return 0;
        case 2:  
            return 1;
        case 4:  
            return 2;
        case 8:  
            return 4;
        default:
            return 8;
    }
}

void GLContext::enableErrorLog(bool bEnable)
{
    s_bErrorLogEnabled = bEnable;
}

void GLContext::checkGPUMemInfoSupport()
{
    if (!m_bCheckedGPUMemInfoExtension) {
        m_bGPUMemInfoSupported = queryOGLExtension("GL_NVX_gpu_memory_info");
        m_bCheckedGPUMemInfoExtension = true;
    }
    if (!m_bGPUMemInfoSupported) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Video memory query not supported on this system.");
    }
}

bool GLContext::isDebugContextSupported() const
{
    if (queryOGLExtension("GL_ARB_debug_output") || queryOGLExtension("GL_KHR_debug")) {
        return true;
    }
    if (isGLES() && isVendor("NVIDIA")) {
        // There is no extension for debug output in gles 2.0, but Linux NVidia
        // supports the functionality anyway. So we activate it :-).
        return true;
    }
    return false;
}

void GLContext::debugLogCallback(GLenum source, GLenum type, GLuint id, 
        GLenum severity, GLsizei length, const GLchar* message, void* userParam) 
{
/*    
    string sSource;
    switch (source) {
        case GL_DEBUG_SOURCE_API_ARB:
            sSource = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
            sSource = "Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
            sSource = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
            sSource = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB:
            sSource = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER_ARB:
            sSource = "Other";
            break;
        default:
            AVG_ASSERT(false);
    }

    string sSeverity;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH_ARB:
            sSeverity = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:
            sSeverity = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW_ARB:
            sSeverity = "Low";
            break;
        default:
            AVG_ASSERT(false);
    }

    string sType;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR_ARB:
            sType = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
            sType = "Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
            sType = "Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB:
            sType = "Portability Issue";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:
            sType = "Performance Issue";
            break;
        case GL_DEBUG_TYPE_OTHER_ARB:
            sType = "Other";
            break;
        default:
            AVG_ASSERT(false);
    }
*/

    // XXX Temporary to clean up NVidia message spam.
#ifndef AVG_ENABLE_EGL
    if (type != GL_DEBUG_TYPE_PERFORMANCE_ARB && s_bErrorLogEnabled) {
#endif
        AVG_LOG_WARNING(message);
        //        dumpBacktrace();
        //        AVG_ASSERT(false);
#ifndef AVG_ENABLE_EGL
    }
#endif
}

}
