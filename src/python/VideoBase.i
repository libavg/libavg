//
// $Id$
// 

%module avg
%{
#include "../player/VideoBase.h"
%}

namespace avg {

class VideoBase
{
    public:
        void play();
        void stop();
        void pause();
        virtual double getFPS() = 0;
    protected:
        VideoBase();
};

}

