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

#include "VersionInfo.h"
#include "../version.h"

namespace avg {

const std::string VersionInfo::getFull()
{
    return std::string(AVG_VERSION_FULL);
}

const std::string VersionInfo::getRelease()
{
    return std::string(AVG_VERSION_RELEASE);
}

const std::string VersionInfo::getBuilder()
{
    return std::string(AVG_VERSION_BUILDER);
}

const std::string VersionInfo::getBuildTime()
{
    return std::string(AVG_VERSION_BUILDTIME);
}

const std::string VersionInfo::getMajor()
{
    return AVG_VERSION_MAJOR;
}

const std::string VersionInfo::getMinor()
{
    return AVG_VERSION_MINOR;
}

const std::string VersionInfo::getMicro()
{
    return AVG_VERSION_MICRO;
}

const std::string VersionInfo::getRevision()
{
    return AVG_VERSION_REVISION;
}

}
