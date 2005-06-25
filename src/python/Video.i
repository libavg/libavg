//
// $Id$
// 

%module avg
%{
#include "../player/Video.h"
%}

%attribute(avg::Video, const std::string&, href, getHRef);
%attribute(avg::Video, bool, loop, getLoop);

namespace avg {

class Video: public VideoBase
{
    public:
        int getNumFrames() const;
        int getCurFrame() const;
        void seekToFrame(int num);
};

}

