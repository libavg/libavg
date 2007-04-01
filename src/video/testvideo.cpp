//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "FFMpegDecoder.h"

#include "../graphics/Filterfliprgb.h"

#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/Profiler.h"

#include <Magick++.h>
#include <stdio.h>
#include <string>

using namespace avg;
using namespace std;

// TODO: 
//  - Seek forward & back (mjpeg & longer movies)
//  - Test getNumFrames
//  - Repeat for other File formats.
//  - Remove testfiles from python tests.
//  - Repeat for async decoder
//  - Test YCbCr420p, YCbCr422
//  - Standard way of saving test bmps
//  - map Magick::Exception to avg::Exception

class DecoderTest: public Test {
    public:
        DecoderTest(bool bSyncDemuxer)
          : Test(getDecoderName(bSyncDemuxer), 2),
            m_bSyncDemuxer(bSyncDemuxer)
        {}

        void runTests()
        {
            try {
                VideoDecoderPtr pDecoder(new FFMpegDecoder());

                pDecoder->open("testfiles/mpeg1-48x48.mpg", OGL_NONE, m_bSyncDemuxer);
                IntPoint FrameSize = pDecoder->getSize();
                TEST(FrameSize == IntPoint(48, 48));
                TEST(pDecoder->getPixelFormat() == B8G8R8X8);
                BitmapPtr pBmp(new Bitmap(FrameSize, B8G8R8X8));

                // Test first two frames.
                pDecoder->renderToBmp(pBmp);
                compareImages(pBmp, "frame1");
                pDecoder->renderToBmp(pBmp);
                compareImages(pBmp, "frame2");
                
                // Read whole file, test last image.
                int NumFrames = 2;
                while(!pDecoder->isEOF()) {
                    pDecoder->renderToBmp(pBmp);
                    NumFrames++;
                }
                TEST(NumFrames == 30);
                // FIXME: Last frame is never decoded (at least for mpegs)
                compareImages(pBmp, "frame30");

                // Test loop.
                pDecoder->seek(0);
                // FIXME: Seek occurs one frame to late.
                pDecoder->renderToBmp(pBmp);
                pDecoder->renderToBmp(pBmp);
                compareImages(pBmp, "frame1");

                pDecoder->close();
            } catch (Magick::Exception & ex) {
                cerr << string(m_IndentLevel+6, ' ') << ex.what() << endl;
                throw;
            }
        }

    private:
        void compareImages(BitmapPtr pBmp, string sFilename) {
            try {
                BitmapPtr pBaselineBmp(new Bitmap("testfiles/baseline/"+sFilename+".png"));
                FilterFlipRGB().applyInPlace(pBaselineBmp);
                if (!(*pBaselineBmp == *pBmp)) {
                    pBmp->save("testfiles/result/"+sFilename+".png");
                    pBaselineBmp->save("testfiles/result/"+sFilename+"_baseline.png");
                    Bitmap DiffBmp(*pBmp);
                    DiffBmp.subtract(&*pBaselineBmp);
                    DiffBmp.save("testfiles/result/"+sFilename+"_diff.png");
                }
                TEST(*pBaselineBmp == *pBmp);
            } catch (Magick::Exception & ex) {
                pBmp->save("testfiles/result/"+sFilename+".png");
                TEST_FAILED("Error loading baseline image: " << ex.what()); 
            }
        }

        string getDecoderName(bool bSyncDemuxer) {
            if (bSyncDemuxer) {
                return string("DecoderTest(Sync demuxer)");
            } else {
                return string("DecoderTest(Async demuxer)");
            }
        }

        bool m_bSyncDemuxer;
};

class VideoTestSuite: public TestSuite {
public:
    VideoTestSuite() 
        : TestSuite("VideoTestSuite")
    {
        addTest(TestPtr(new DecoderTest(true)));
        addTest(TestPtr(new DecoderTest(false)));
    }
};


void deleteOldResultImages() 
{
    // TODO
}

int main(int nargs, char** args)
{
    ThreadProfilerPtr pThreadProfiler = ThreadProfilerPtr(new ThreadProfiler("Main"));
    Profiler::get().registerThreadProfiler(pThreadProfiler);

    deleteOldResultImages();

    VideoTestSuite Suite;
    Suite.runTests();
    bool bOK = Suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

