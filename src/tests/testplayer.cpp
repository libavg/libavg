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

#include "../player/Player.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/Logger.h"

#include "../graphics/GLConfig.h"
#include "../graphics/GLContext.h"
#include "../graphics/ShaderRegistry.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>

#ifdef WIN32
#include <direct.h>
#endif

using namespace avg;
using namespace std;

class PlayerTest: public Test {
public:
    PlayerTest()
        : Test("PlayerTest", 2)
    {
    }

    void runTests()
    {
        Player player;
        player.loadString(
        "    <?xml version=\"1.0\"?>"
        "    <avg width=\"160\" height=\"120\">"
        "      <words text=\"foo\"/>"
        "    </avg>"
        );
        player.setOGLOptions(false, true, 1, GLConfig::AUTO, true);
        GLContext::enableErrorChecks(true);
        player.disablePython();
        if (!getenv("AVG_CONSOLE_TEST")) {
#ifdef WIN32
            char sz[1024];
            _getcwd(sz, 1024);
            cerr << "Current directory: " << sz << endl;
#endif
            ShaderRegistry::setShaderPath(
                    getSrcDirName()+"../graphics/shaders");
            player.initPlayback();
            player.doFrame(false);
            player.cleanup(false);
        }
        try {
            throw bad_cast();
        } catch (bad_cast&) {

        }
    }
};

class PlayerTestSuite: public TestSuite {
public:
    PlayerTestSuite(const std::string& sSrcDir)
        : TestSuite("PlayerTestSuite", sSrcDir)
    {
        addTest(TestPtr(new PlayerTest));
    }
};


int main(int nargs, char** args)
{
    assert(nargs == 2);
    PlayerTestSuite suite(args[1]);
    suite.runTests();
    bool bOK = suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

