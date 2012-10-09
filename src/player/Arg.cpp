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

#include "Arg.h"
#include "FontStyle.h"

#include <string>

using namespace std;

namespace avg {
// Explicit template instantiation. See Arg.h for comments.
#ifndef _WIN32
template class Arg<int>;
template class Arg<bool>;
template class Arg<float>;
template class Arg<string>;
template class Arg<glm::vec2>;
template class Arg<glm::vec3>;
template class Arg<glm::ivec3>;
template class Arg<std::vector<float> >;
template class Arg<std::vector<int> >;
template class Arg<vector<glm::vec2> >;
template class Arg<vector<glm::ivec3> >;
template class Arg<FontStyle>;
#endif
}
