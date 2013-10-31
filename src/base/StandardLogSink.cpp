//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2012 Ulrich von Zadow
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

#include "StandardLogSink.h"
#include "Logger.h"

#include <iostream>
#include <iomanip>

using namespace std;

namespace avg
{

StandardLogSink::StandardLogSink()
{

}

StandardLogSink::~StandardLogSink()
{

}

void StandardLogSink::logMessage(const tm* pTime, unsigned millis,
        const category_t& category, severity_t severity, const UTF8String& sMsg)
{
    char timeString[256];
    strftime(timeString, sizeof(timeString), "%y-%m-%d %H:%M:%S", pTime);
    cerr << "[" << timeString << "." << 
        setw(3) << setfill('0') << millis << setw(0) << "][";
    cerr << setw(4) << setfill('.') << Logger::severityToString(severity) << "][";
    cerr << setw(9) << setfill('.') << category << "] : " << sMsg << endl;
    cerr.flush();
}

}
