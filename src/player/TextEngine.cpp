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

#include "TextEngine.h"

#include "../base/Logger.h"
#include "../base/OSHelper.h"
#include "../base/Exception.h"
#include "../base/FileHelper.h"
#include "../base/StringHelper.h"

#include <algorithm>
#include <cairo.h>
#include <pango/pangocairo.h>

namespace avg {

using namespace std;

TextEngine& TextEngine::get(bool bHint) 
{
    if (bHint) {
        static TextEngine s_Instance(true);
        return s_Instance;
    } else {
        static TextEngine s_Instance(false);
        return s_Instance;
    }
}


TextEngine::TextEngine(bool bHint)
    : m_bHint(bHint)
{
    m_sFontDirs.push_back("fonts/");
    init();
}

TextEngine::~TextEngine()
{
    deinit();
}

void TextEngine::init()
{
    m_pFontMap = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
    pango_cairo_font_map_set_resolution(PANGO_CAIRO_FONT_MAP(m_pFontMap), 72.0);

#if PANGO_VERSION > PANGO_VERSION_ENCODE(1,22,0)
    m_pPangoContext = pango_font_map_create_context(m_pFontMap);
#else
    m_pPangoContext = pango_cairo_font_map_create_context(PANGO_CAIRO_FONT_MAP(m_pFontMap));
#endif

    cairo_font_options_t *pFontOptions;
    pFontOptions = cairo_font_options_create();
    cairo_font_options_set_antialias(pFontOptions, CAIRO_ANTIALIAS_DEFAULT);
    if (m_bHint) {
        cairo_font_options_set_hint_style(pFontOptions,
                CAIRO_HINT_STYLE_SLIGHT);
    } else {
        cairo_font_options_set_hint_style(pFontOptions,
                CAIRO_HINT_STYLE_NONE);
    }
    cairo_font_options_set_hint_metrics(pFontOptions, CAIRO_HINT_METRICS_ON);
    pango_cairo_context_set_font_options(m_pPangoContext, pFontOptions);
    cairo_font_options_destroy(pFontOptions);

    pango_context_set_language(m_pPangoContext,
            pango_language_from_string ("en_US"));
    pango_context_set_base_dir(m_pPangoContext, PANGO_DIRECTION_LTR);

    initFonts();

    string sOldLang = "";
    getEnv("LC_CTYPE", sOldLang);
    setEnv("LC_CTYPE", "en-us");
    pango_font_map_list_families(m_pFontMap, &m_ppFontFamilies,
            &m_NumFontFamilies);
    setEnv("LC_CTYPE", sOldLang);
    for (int i = 0; i < m_NumFontFamilies; ++i) {
        m_sFonts.push_back(pango_font_family_get_name(m_ppFontFamilies[i]));
    }
    sort(m_sFonts.begin(), m_sFonts.end());
}

void TextEngine::deinit()
{
    g_object_unref(m_pFontMap);
    g_free(m_ppFontFamilies);
    g_object_unref(m_pPangoContext);
    m_sFonts.clear();
}

void TextEngine::addFontDir(const std::string& sDir)
{
    deinit();
    m_sFontDirs.push_back(sDir);
    init();
}

PangoContext * TextEngine::getPangoContext()
{
    return m_pPangoContext;
}

const vector<string>& TextEngine::getFontFamilies()
{
    return m_sFonts;
}

const vector<string>& TextEngine::getFontVariants(const string& sFontName)
{
    PangoFontFamily * pCurFamily = getFontFamily(sFontName);
    PangoFontFace ** ppFaces;
    int numFaces;
    pango_font_family_list_faces (pCurFamily, &ppFaces, &numFaces);
    static vector<string> sVariants;
    sVariants.clear();
    for (int i = 0; i < numFaces; ++i) {
        sVariants.push_back(pango_font_face_get_face_name(ppFaces[i]));
    }
    g_free(ppFaces);
    return sVariants;
}

PangoFontDescription * TextEngine::getFontDescription(const string& sFamily, 
        const string& sVariant)
{
    PangoFontDescription* pDescription;
    FontDescriptionCache::iterator it;
    it = m_FontDescriptionCache.find(pair<string, string>(sFamily, sVariant));
    if (it == m_FontDescriptionCache.end()) {
        PangoFontFamily * pFamily;
        bool bFamilyFound = true;
        try {
            pFamily = getFontFamily(sFamily);
        } catch (Exception&) {
            if (m_sFontsNotFound.find(sFamily) == m_sFontsNotFound.end()) {
                AVG_LOG_WARNING("Could not find font face " << sFamily << 
                        ". Using sans instead.");
                m_sFontsNotFound.insert(sFamily);
            }
            bFamilyFound = false;
            pFamily = getFontFamily("sans");
        }
        PangoFontFace ** ppFaces;
        int numFaces;
        pango_font_family_list_faces(pFamily, &ppFaces, &numFaces);
        PangoFontFace * pFace = 0;
        if (sVariant == "") {
            pFace = ppFaces[0];
        } else {
            for (int i = 0; i < numFaces; ++i) {
                if (equalIgnoreCase(pango_font_face_get_face_name(ppFaces[i]), sVariant)) {
                    pFace = ppFaces[i];
                }
            }
        }
        if (!pFace) {
            pFace = ppFaces[0];
            if (bFamilyFound) {
                pair<string, string> variant(sFamily, sVariant);
                if (m_VariantsNotFound.find(variant) == m_VariantsNotFound.end()) {
                    m_VariantsNotFound.insert(variant);
                    AVG_LOG_WARNING("Could not find font variant " 
                            << sFamily << ":" << sVariant << ". Using " <<
                            pango_font_face_get_face_name(pFace) << " instead.");
                }
            }
        }
        g_free(ppFaces);
        pDescription = pango_font_face_describe(pFace);
        m_FontDescriptionCache[pair<string, string>(sFamily, sVariant)] =
                pDescription;
    } else {
        pDescription = it->second;
    }
    return pango_font_description_copy(pDescription);
}

void GLibLogFunc(const gchar *log_domain, GLogLevelFlags log_level, 
        const gchar *message, gpointer unused_data)
{
//TODO: Make this use correct AVG_LOG_LEVEL function
#ifndef WIN32
    string s = "Pango ";
    if (log_level & G_LOG_LEVEL_ERROR) {
        s += message;
        AVG_LOG_ERROR(s);
        return;
    } else if (log_level & G_LOG_LEVEL_CRITICAL) {
        s += message;
        AVG_LOG_ERROR(s);
        AVG_ASSERT(false);
    } else if (log_level & G_LOG_LEVEL_WARNING) {
        s += message;
        AVG_LOG_WARNING(s);
        return;
    } else if (log_level & G_LOG_LEVEL_MESSAGE) {
        s += (string("message: ") + message);
        AVG_LOG_INFO(s);
        return;
    } else if (log_level & G_LOG_LEVEL_INFO) {
        s += message;
        AVG_LOG_INFO(s);
        return;
    } else if (log_level & G_LOG_LEVEL_DEBUG) {
        s += message;
        AVG_TRACE(Logger::category::NONE, Logger::severity::DEBUG, s);
        return;
    }
    s += message;
    AVG_LOG_WARNING(s);
#endif
}

void TextEngine::initFonts()
{
    std::vector<std::string> fontConfPathPrefixList;
#ifndef WIN32
    fontConfPathPrefixList.push_back("/");
    fontConfPathPrefixList.push_back("/usr/local/");
    fontConfPathPrefixList.push_back("/opt/local/");
#endif
    fontConfPathPrefixList.push_back(getAvgLibPath());
    fontConfPathPrefixList.push_back("../../");

    std::string sFontConfPath;
    for (size_t i = 0; i < fontConfPathPrefixList.size(); ++i) {
        sFontConfPath = fontConfPathPrefixList[i] + "etc/fonts/fonts.conf";
        if (fileExists(sFontConfPath)) {
            break;
        }
    }

    FcConfig * pConfig = FcConfigCreate();
    int ok = (int)FcConfigParseAndLoad(pConfig, 
            (const FcChar8 *)(sFontConfPath.c_str()), true);
    checkFontError(ok, string("Font error: could not load config file ")+sFontConfPath);
    ok = (int)FcConfigBuildFonts(pConfig);
    checkFontError(ok, string("Font error: FcConfigBuildFonts failed."));
    ok = (int)FcConfigSetCurrent(pConfig);
    checkFontError(ok, string("Font error: FcConfigSetCurrent failed."));
    for(std::vector<std::string>::const_iterator it = m_sFontDirs.begin();
            it != m_sFontDirs.end(); ++it)
    {
        FcConfigAppFontAddDir(pConfig, (const FcChar8 *)it->c_str());
    }
    /*
       FcStrList * pCacheDirs = FcConfigGetCacheDirs(pConfig);
       FcChar8 * pDir;
       do {
       pDir = FcStrListNext(pCacheDirs);
       if (pDir) {
       cerr << pDir << endl;
       }
       } while (pDir);
     */
    g_log_set_default_handler(GLibLogFunc, 0);
}

PangoFontFamily * TextEngine::getFontFamily(const string& sFamily)
{
    PangoFontFamily * pFamily = 0;
    AVG_ASSERT(m_NumFontFamilies != 0);
    for (int i=0; i<m_NumFontFamilies; ++i) {
        if (equalIgnoreCase(pango_font_family_get_name(m_ppFontFamilies[i]), sFamily)) {
            pFamily = m_ppFontFamilies[i];
        }
    }
    if (!pFamily) {
        throw(Exception(AVG_ERR_INVALID_ARGS, 
                "getFontFamily: Font family "+sFamily+" not found."));
    }
    return pFamily;
}

void TextEngine::checkFontError(int ok, const string& sMsg)
{
    if (ok == 0) {
        throw Exception(AVG_ERR_FONT_INIT_FAILED, sMsg);
    }
}

}
