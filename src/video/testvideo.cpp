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

#include "../base/TimeSource.h"
#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/Profiler.h"

#include <Magick++.h>
#include <stdio.h>
#include <string>

using namespace avg;
using namespace std;

// TODO: 
//  - Seek forward & back (longer movies)
//  - Test getNumFrames
//  - Repeat for other File formats.
//  - Remove testfiles from python tests.
//  - Test YCbCr420p, YCbCr422

class DecoderTest: public Test {
    public:
        DecoderTest(bool bThreadedDemuxer)
          : Test(getDecoderName(bThreadedDemuxer), 2),
            m_bThreadedDemuxer(bThreadedDemuxer)
        {}

        void runTests()
        {
            basicFileTest("mpeg1-48x48.mpg", 30);
            basicFileTest("mjpeg-48x48.avi", 202);
            seekTest("mjpeg-48x48.avi");
        }

    private:

        void basicFileTest(const string& sFilename, int ExpectedNumFrames) 
        {
            try {
                cerr << "    Testing " << sFilename << endl;
                VideoDecoderPtr pDecoder(new FFMpegDecoder());

                pDecoder->open(string("testfiles/")+sFilename, OGL_NONE, m_bThreadedDemuxer);
                IntPoint FrameSize = pDecoder->getSize();
                TEST(FrameSize == IntPoint(48, 48));
                TEST(pDecoder->getPixelFormat() == B8G8R8X8);
                BitmapPtr pBmp(new Bitmap(FrameSize, B8G8R8X8));

                // Test first two frames.
                pDecoder->renderToBmp(pBmp);
                compareImages(pBmp, sFilename+"_1");
                pDecoder->renderToBmp(pBmp);
                compareImages(pBmp, sFilename+"_2");
                
                // Read whole file, test last image.
                int NumFrames = 1;
                while(!pDecoder->isEOF()) {
                    pDecoder->renderToBmp(pBmp);
                    NumFrames++;
                }
                TEST(NumFrames == ExpectedNumFrames);
                compareImages(pBmp, sFilename+"_end");

                // Test loop.
                pDecoder->seek(0);
                pDecoder->renderToBmp(pBmp);
                compareImages(pBmp, sFilename+"_loop");

                pDecoder->close();
            } catch (Magick::Exception & ex) {
                cerr << string(m_IndentLevel+6, ' ') << ex.what() << endl;
                throw;
            }
        }

        void seekTest(const string& sFilename)
        {
            cerr << "    Testing " << sFilename << " (seek)" << endl;
            VideoDecoderPtr pDecoder(new FFMpegDecoder());

            pDecoder->open(string("testfiles/")+sFilename, OGL_NONE, m_bThreadedDemuxer);

            IntPoint FrameSize = pDecoder->getSize();
            BitmapPtr pBmp(new Bitmap(FrameSize, B8G8R8X8));

            pDecoder->seek(100);
            pDecoder->renderToBmp(pBmp);
            compareImages(pBmp, sFilename+"_100");

            pDecoder->seek(53);
            pDecoder->renderToBmp(pBmp);
            compareImages(pBmp, sFilename+"_53");

            pDecoder->close();
        }

        void compareImages(BitmapPtr pBmp, const string& sFilename)
        {
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

        string getDecoderName(bool bThreadedDemuxer) {
            if (bThreadedDemuxer) {
                return string("DecoderTest(Threaded demuxer)");
            } else {
                return string("DecoderTest(Sync demuxer)");
            }
        }

        bool m_bThreadedDemuxer;
};

class VideoTestSuite: public TestSuite {
public:
    VideoTestSuite() 
        : TestSuite("VideoTestSuite")
    {
        addTest(TestPtr(new DecoderTest(false)));
        addTest(TestPtr(new DecoderTest(true)));
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

