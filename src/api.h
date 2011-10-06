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
//  Original author of this file is Jan Boelsche (regular.gonzales@googlemail.com).
//

#ifndef _API_H_
#define _API_H_

#ifdef _WIN32 // in declspec land
#pragma warning(disable: 4251)
#define AVG_PLUGIN_API extern "C" __declspec(dllexport)
#ifdef AVG_PLUGIN
#define AVG_API __declspec(dllimport)
#define AVG_TEMPLATE_API
#else
#define AVG_API __declspec(dllexport)
#define AVG_TEMPLATE_API __declspec(dllexport)
#endif
#else // not _WIN32, plain and simple
#define AVG_API
#define AVG_TEMPLATE_API
#define AVG_PLUGIN_API extern "C"
#endif

#endif

