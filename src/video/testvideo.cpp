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

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif
#include <Magick++.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "AsyncVideoDecoder.h"

#include "../graphics/Filterfliprgba.h"
#include "../graphics/Filterfliprgb.h"

#include "../base/StringHelper.h"
#include "../base/TimeSource.h"
#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/Profiler.h"
#include "../base/Directory.h"
#include "../base/DirEntry.h"

#include <stdio.h>
#include <string>
#include <sstream>

using namespace avg;
using namespace std;

// TODO: 
//  - Seek forward & back (longer movies)
//  - Test getNumFrames
//  - Repeat for other File formats.
//  - Test YCbCr420p, YCbCr422


class DecoderTest: public Test {
    public:
        DecoderTest(const string& sClassName, bool bThreadedDecoder, bool bThreadedDemuxer)
          : Test(sClassName+getDecoderName(bThreadedDecoder, bThreadedDemuxer), 2),
            m_bThreadedDecoder(bThreadedDecoder),
            m_bThreadedDemuxer(bThreadedDemuxer)
        {}

    protected:
        bool isDemuxerThreaded() 
        {
            return m_bThreadedDemuxer;
        }

        VideoDecoderPtr createDecoder() 
        {
            VideoDecoderPtr pDecoder;
            pDecoder = VideoDecoderPtr(new FFMpegDecoder());
            if (m_bThreadedDecoder) {
                pDecoder = VideoDecoderPtr(new AsyncVideoDecoder(pDecoder));
            }
            return pDecoder;
        }

        void compareImages(BitmapPtr pBmp, const string& sFilename)
        {
            BitmapPtr pBaselineBmp;
            try {
                pBaselineBmp = BitmapPtr(new Bitmap(
                        getSrcDir()+"testfiles/baseline/"+sFilename+".png"));
            } catch (Magick::Exception & ex) {
                TEST_FAILED("Error loading baseline image: " << ex.what()); 
                pBmp->save(getSrcDir()+"testfiles/result/"+sFilename+".png");
                return;
            }
            FilterFlipRGB().applyInPlace(pBaselineBmp);
#ifdef __BIG_ENDIAN__
            FilterFlipRGBA().applyInPlace(pBmp);
#endif
            int DiffPixels = pBaselineBmp->getNumDifferentPixels(*pBmp);
            if (DiffPixels > 0) {
                TEST_FAILED("Error: Decoded image differs from baseline '" << 
                        sFilename << "'. " << DiffPixels << " different pixels.");
                try {
                    pBmp->save(getSrcDir()+"testfiles/result/"+sFilename+".png");
                    BitmapPtr pOrigBmp(new Bitmap(getSrcDir()+"testfiles/baseline/"+sFilename+".png"));
                    pOrigBmp->save(getSrcDir()+"testfiles/result/"+sFilename+"_baseline.png");
                    Bitmap DiffBmp(*pBmp);
                    DiffBmp.subtract(&*pBaselineBmp);
                    DiffBmp.save(getSrcDir()+"testfiles/result/"+sFilename+"_diff.png");
                } catch (Magick::Exception & ex) {
                    TEST_FAILED("Error saving result image: " << ex.what()); 
                    return;
                }
            }
        }

        AudioBufferPtr createAudioBuffer(int NumFrames)
        {
            return AudioBufferPtr(new AudioBuffer(NumFrames, *getAudioParams()));
        }

        const AudioParams* getAudioParams()
        {
            static AudioParams AP(44100, 2, 256);
            return &AP;
        }

    private:
        string getDecoderName(bool bThreadedDecoder, bool bThreadedDemuxer) {
            string sName = "(";
            if (bThreadedDecoder) {
                sName += "Threaded decoder, ";
            } else {
                sName += "Sync decoder, ";
            }
            if (bThreadedDemuxer) {
                return sName+string("Threaded demuxer)");
            } else {
                return sName+string("Sync demuxer)");
            }
        }

        bool m_bThreadedDecoder;
        bool m_bThreadedDemuxer;
};

class VideoDecoderTest: public DecoderTest {
    public:
        VideoDecoderTest(bool bThreadedDecoder, bool bThreadedDemuxer)
            :DecoderTest("VideoDecoderTest", bThreadedDecoder, bThreadedDemuxer)
        {}

        void runTests()
        {
            basicFileTest("mpeg1-48x48.mpg", 30);
            basicFileTest("mjpeg-48x48.avi", 202);
            testSeeks("mjpeg-48x48.avi");
        }

    private:
        void basicFileTest(const string& sFilename, int ExpectedNumFrames) 
        {
            try {
                cerr << "    Testing " << sFilename << endl;

                VideoDecoderPtr pDecoder = createDecoder();
                pDecoder->open(getSrcDir()+"testfiles/"+sFilename, getAudioParams(),
                        OGL_NONE, isDemuxerThreaded());
                IntPoint FrameSize = pDecoder->getSize();
                TEST(FrameSize == IntPoint(48, 48));
                TEST(pDecoder->getPixelFormat() == B8G8R8X8);
                TEST(pDecoder->getNominalFPS() != 0);
                BitmapPtr pBmp(new Bitmap(FrameSize, B8G8R8X8));

                // Test first two frames.
                pDecoder->renderToBmp(pBmp, -1);
                compareImages(pBmp, sFilename+"_1");
                pDecoder->renderToBmp(pBmp, -1);
                compareImages(pBmp, sFilename+"_2");
                pDecoder->close();
                
                readWholeFile(sFilename, 1, ExpectedNumFrames); 
                readWholeFile(sFilename, 0.5, ExpectedNumFrames); 
                readWholeFile(sFilename, 2, ExpectedNumFrames/2); 
            } catch (Magick::Exception & ex) {
                cerr << string(m_IndentLevel+6, ' ') << ex.what() << endl;
                throw;
            }
        }

        void testSeeks(const string& sFilename)
        {
            cerr << "    Testing " << sFilename << " (seek)" << endl;

            VideoDecoderPtr pDecoder = createDecoder();
            pDecoder->open(getSrcDir()+"testfiles/"+sFilename, getAudioParams(),
                    OGL_NONE, isDemuxerThreaded());

            // Seek forward
            testSeek(100, sFilename, pDecoder);
            // Seek backward
            testSeek(53, sFilename, pDecoder);
            // Seek to last frame
            testSeek(201, sFilename, pDecoder);

            pDecoder->close();
        }

        void testSeek(int FrameNum, const string& sFilename, VideoDecoderPtr pDecoder)
        {
            IntPoint FrameSize = pDecoder->getSize();

            BitmapPtr pBmp(new Bitmap(FrameSize, B8G8R8X8));
            pDecoder->seek((long long)(FrameNum*1000/pDecoder->getNominalFPS()));
            pDecoder->renderToBmp(pBmp, -1);
            compareImages(pBmp, sFilename+"_"+toString(FrameNum));

        }

        void readWholeFile(const string& sFilename, 
                double SpeedFactor, int ExpectedNumFrames)
        {
            // Read whole file, test last image.
            VideoDecoderPtr pDecoder = createDecoder();
            pDecoder->open(getSrcDir()+"testfiles/"+sFilename, getAudioParams(),
                    OGL_NONE, isDemuxerThreaded());
            IntPoint FrameSize = pDecoder->getSize();
            BitmapPtr pBmp(new Bitmap(FrameSize, B8G8R8X8));
            double TimePerFrame = (1000.0/pDecoder->getFPS())*SpeedFactor;
            int NumFrames = 0;
            double CurTime = 0;

            while(!pDecoder->isEOF()) {
                FrameAvailableCode FrameAvailable = 
                        pDecoder->renderToBmp(pBmp, (long long)CurTime);
                if (FrameAvailable == FA_NEW_FRAME) {
//                    stringstream ss;
//                    ss << "testfiles/result/" << sFilename << NumFrames << ".png";
//                    pBmp->save(ss.str());
                    NumFrames++;

                } else {
                    msleep(0);
                }
                if (FrameAvailable == FA_NEW_FRAME || FrameAvailable == FA_USE_LAST_FRAME) { 
                    CurTime += TimePerFrame;
                }
            }
//            cerr << "NumFrames: " << NumFrames << ", ExpectedNumFrames: " << ExpectedNumFrames << endl;
            TEST(NumFrames == ExpectedNumFrames);
            if (SpeedFactor == 1) {
                compareImages(pBmp, sFilename+"_end");
            }
            
            // Test loop.
            pDecoder->seek(0);
            pDecoder->renderToBmp(pBmp, -1);
            compareImages(pBmp, sFilename+"_loop");

            pDecoder->close();
        }

};

class AudioDecoderTest: public DecoderTest {
    public:
        AudioDecoderTest(bool bThreadedDecoder, bool bThreadedDemuxer)
          : DecoderTest("AudioDecoderTest", bThreadedDecoder, bThreadedDemuxer)
        {}

        void runTests()
        {
            testOneFile("22.050Hz_16bit_mono.wav");

            testOneFile("44.1kHz_16bit_mono.wav");
            testOneFile("44.1kHz_16bit_stereo.wav");
            testOneFile("44.1kHz_24bit_mono.wav");
            testOneFile("44.1kHz_24bit_stereo.wav");

            testOneFile("48kHz_16bit_mono.wav");
            testOneFile("48kHz_16bit_stereo.wav");
            testOneFile("48kHz_24bit_mono.wav");
            testOneFile("48kHz_24bit_stereo.wav");

            testOneFile("44.1kHz_16bit_stereo.aif");
            testOneFile("44.1kHz_stereo.mp3");
        }

    private:
        void testOneFile(const string& sFilename)
        {
            try {
                cerr << "    Testing " << sFilename << endl;
                
                {
                    cerr << "      Reading complete file." << endl;
                    VideoDecoderPtr pDecoder = createDecoder();
                    pDecoder->open(getSrcDir()+"testfiles/"+sFilename, getAudioParams(),
                            OGL_NONE, isDemuxerThreaded());
                    pDecoder->setVolume(0.5);
                    TEST(pDecoder->getVolume() == 0.5);
                    int TotalFramesDecoded = 0;
                    bool bCheckTimestamps = (sFilename.find(".ogg") == string::npos &&
                            sFilename.find(".mp3") == string::npos);
                    readAudioToEOF(pDecoder, TotalFramesDecoded, bCheckTimestamps);

                    // Check if we've decoded the whole file.
                    int FramesInDuration = int(pDecoder->getDuration()*44100/1000);
//                    cerr << "FramesDecoded: " << TotalFramesDecoded << endl;
//                    cerr << "FramesInDuration: " << FramesInDuration << endl;
                    TEST (abs(TotalFramesDecoded-FramesInDuration) < 45);
                }
                {
                    cerr << "      Seek test." << endl;
                    VideoDecoderPtr pDecoder = createDecoder();
                    pDecoder->open(getSrcDir()+"testfiles/"+sFilename, getAudioParams(),
                            OGL_NONE, isDemuxerThreaded());
                    long long Duration = pDecoder->getDuration();
                    pDecoder->seek(Duration/2);
                    AudioBufferPtr pAudioBuffer = createAudioBuffer(4);
                    pDecoder->fillAudioBuffer(pAudioBuffer);
                    // 60 ms accuracy for seeks.
                    TEST(abs(long(Duration/2-pDecoder->getCurTime())) < 60); 
                    int TotalFramesDecoded = 4;

                    readAudioToEOF(pDecoder, TotalFramesDecoded, false);
                    if (sFilename.find(".mp3") == string::npos)
                    {
                        // Check if we've decoded half the file.
                        // TODO: Find out why there are problems with this
                        // for mp3 files.
                        int FramesInDuration = int(pDecoder->getDuration()*44100/1000);
//                        cerr << "FramesDecoded: " << FramesDecoded << endl;
//                        cerr << "FramesInDuration: " << FramesInDuration << endl;
                        TEST (abs(TotalFramesDecoded-FramesInDuration/2) < 45);
                    }

                }

            } catch (Magick::Exception & ex) {
                cerr << string(m_IndentLevel+6, ' ') << ex.what() << endl;
                throw;
            }
        }

        void readAudioToEOF(VideoDecoderPtr pDecoder, int& TotalFramesDecoded,
                bool bCheckTimestamps) 
        {
            int NumWrongTimestamps = 0;
            while(!pDecoder->isEOF()) {
                AudioBufferPtr pBuffer = createAudioBuffer(256);
                int FramesDecoded = 0;
                while (FramesDecoded == 0 && !pDecoder->isEOF()) {
                    FramesDecoded = pDecoder->fillAudioBuffer(pBuffer);
//                    cerr << "FramesDecoded: " << FramesDecoded << endl;
                    msleep(0);
                }
                TotalFramesDecoded += FramesDecoded;
                long long CurTime = int(TotalFramesDecoded/44.1);
                if (abs(long(CurTime-pDecoder->getCurTime())) > 20) {
                    NumWrongTimestamps++;
                }
//                cerr << CurTime << "->" << pDecoder->getCurTime() << endl;
            }
            if (bCheckTimestamps) {
                if (NumWrongTimestamps>0) {
                    TEST_FAILED(NumWrongTimestamps << " wrong timestamps.");
                }
            }
        }
};

class AVDecoderTest: public DecoderTest {
    public:
        AVDecoderTest(bool bThreadedDecoder, bool bThreadedDemuxer)
          : DecoderTest("AVDecoderTest", bThreadedDecoder, bThreadedDemuxer)
        {}

        void runTests()
        {
            basicFileTest("mpeg1-48x48-sound.avi", 30);
        }

    private:
        void basicFileTest(const string& sFilename, int ExpectedNumFrames)
        {
            VideoDecoderPtr pDecoder = createDecoder();
            pDecoder->open(getSrcDir()+"testfiles/"+sFilename, getAudioParams(),
                    OGL_NONE, isDemuxerThreaded());
            TEST(pDecoder->getNominalFPS() != 0);
            IntPoint FrameSize = pDecoder->getSize();
            BitmapPtr pBmp(new Bitmap(FrameSize, B8G8R8X8));
            int NumFrames = 0;
            int TotalFramesDecoded = 0;
            int curTime = 0;
            
            while(!pDecoder->isEOF()) {
                FrameAvailableCode FrameAvailable;
                do {
                    FrameAvailable = pDecoder->renderToBmp(pBmp, curTime);
                    msleep(0);
                } while (FrameAvailable == FA_STILL_DECODING);
                if (FrameAvailable == FA_NEW_FRAME) {
//                    stringstream ss;
//                    ss << "testfiles/result/" << sFilename << NumFrames << ".png";
//                    pBmp->save(ss.str());
                    NumFrames++;
                }
                if (isDemuxerThreaded()) {
                    AudioBufferPtr pBuffer = createAudioBuffer(256);
                    int FramesDecoded = 0;
                    while (FramesDecoded == 0 && !pDecoder->isEOF(SS_AUDIO)) {
                        FramesDecoded = pDecoder->fillAudioBuffer(pBuffer);
                        msleep(0);
                    }
                    TotalFramesDecoded += FramesDecoded;
//                    cerr << "FramesDecoded: " << FramesDecoded << endl;
                }
                curTime += 1000/pDecoder->getFPS();
            }
            TEST(pDecoder->isEOF(SS_VIDEO));
//            cerr << "NumFrames: " << NumFrames << endl;
            TEST(NumFrames == ExpectedNumFrames);
            compareImages(pBmp, sFilename+"_end");

            if (isDemuxerThreaded()) {
                // Check if audio length was ok.
                int FramesInDuration = int(pDecoder->getDuration()*44100/1000);
//            cerr << "FramesDecoded: " << FramesDecoded << ", FramesInDuration: " << FramesInDuration << endl;
                TEST (abs(TotalFramesDecoded-FramesInDuration) < 45);
            }

            // Test loop.
            pDecoder->seek(0);
            pDecoder->renderToBmp(pBmp, -1);
            compareImages(pBmp, sFilename+"_loop");

            pDecoder->close();
        }
};


class VideoTestSuite: public TestSuite {
public:
    VideoTestSuite() 
        : TestSuite("VideoTestSuite")
    {
        addTest(TestPtr(new VideoDecoderTest(false, false)));
        addTest(TestPtr(new VideoDecoderTest(false, true)));
        addTest(TestPtr(new VideoDecoderTest(true, true)));

        addTest(TestPtr(new AudioDecoderTest(false, true)));
        addTest(TestPtr(new AudioDecoderTest(true, true)));

        addTest(TestPtr(new AVDecoderTest(false, false)));
        addTest(TestPtr(new AVDecoderTest(false, true)));
        addTest(TestPtr(new AVDecoderTest(true, true)));
    }
};


void deleteOldResultImages() 
{
    string sDirName("testfiles/result/");
    Directory Dir(sDirName);
    int err = Dir.open(true);
    if (err) {
        cerr << "Creating directory " << sDirName << " failed." << strerror(err) << endl;
    } else {
        cerr << "Deleting files in " << sDirName << endl;
        DirEntryPtr pEntry = Dir.getNextEntry();
        while (pEntry) {
            if (pEntry->getName()[0] != '.') {
                pEntry->remove();
            }
            pEntry = Dir.getNextEntry();
        }
    }
}

int main(int nargs, char** args)
{
    ThreadProfilerPtr pThreadProfiler = ThreadProfilerPtr(new ThreadProfiler("Main"));
    Profiler::get().registerThreadProfiler(pThreadProfiler);

    deleteOldResultImages();

    VideoTestSuite Suite;
    bool bOk;
    
    Suite.runTests();
    bOk = Suite.isOk();
    if (bOk) {
        return 0;
    } else {
        return 1;
    }
/*    
    while(true) {
        Suite.runTests();
        bOk = Suite.isOk();
        if (!bOk) {
            return 1;
        }
    }
*/    
}

