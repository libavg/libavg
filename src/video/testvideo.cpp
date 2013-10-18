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

#include "AsyncVideoDecoder.h"
#include "SyncVideoDecoder.h"
#ifdef AVG_ENABLE_VDPAU
#include "VDPAUDecoder.h"
#endif

#include "../graphics/Filterfliprgba.h"
#include "../graphics/Filterfliprgb.h"
#include "../graphics/GraphicsTest.h"
#include "../graphics/BitmapLoader.h"

#include "../base/StringHelper.h"
#include "../base/TimeSource.h"
#include "../base/TestSuite.h"
#include "../base/Exception.h"
#include "../base/ThreadProfiler.h"
#include "../base/Directory.h"
#include "../base/DirEntry.h"

#include <string>
#include <sstream>
#include <cmath>

#include <glib-object.h>

using namespace avg;
using namespace std;
using namespace boost;

class DecoderTest: public GraphicsTest {
    public:
        DecoderTest(const string& sClassName, bool bThreaded, 
                bool bUseHardwareAcceleration)
          : GraphicsTest(sClassName+getDecoderName(bThreaded, bUseHardwareAcceleration),
                2),
            m_bThreaded(bThreaded),
            m_bUseHardwareAcceleration(bUseHardwareAcceleration)
        {}

    protected:
        bool isThreaded() 
        {
            return m_bThreaded;
        }

        bool useHardwareAcceleration()
        {
            return m_bUseHardwareAcceleration;
        }

        VideoDecoderPtr createDecoder() 
        {
            VideoDecoderPtr pDecoder;
            if (m_bThreaded) {
                pDecoder = VideoDecoderPtr(new AsyncVideoDecoder(8));
            } else {
                pDecoder = VideoDecoderPtr(new SyncVideoDecoder());
            }

            return pDecoder;
        }

        const AudioParams* getAudioParams()
        {
            static AudioParams AP(44100, 2, 256);
            return &AP;
        }

        int processAudioMsg(AudioMsgQueuePtr pMsgQ, AudioMsgQueuePtr pStatusQ)
        {
            AudioMsgPtr pMsg = pMsgQ->pop(false);
            if (pMsg) {
                switch (pMsg->getType()) {
                    case AudioMsg::AUDIO: {
                        AudioBufferPtr pBuffer = pMsg->getAudioBuffer();
                        AudioMsgPtr pStatusMsg(new AudioMsg);
                        pStatusMsg->setAudioTime(pMsg->getAudioTime());
                        pStatusQ->push(AudioMsgPtr(pStatusMsg));
                        return pBuffer->getNumFrames();
                    }
                    case AudioMsg::SEEK_DONE: {
                        AudioMsgPtr pStatusMsg(new AudioMsg);
                        pStatusMsg->setSeekDone(pMsg->getSeekSeqNum(),
                                pMsg->getSeekTime());
                        pStatusQ->push(AudioMsgPtr(pStatusMsg));
                        return -1;
                    }
                    default:
                        pStatusQ->push(pMsg);
                        return 0;
                }
            } else {
                return 0;
            }
        }

        void processAudioSeek(AudioMsgQueuePtr pMsgQ, AudioMsgQueuePtr pStatusQ)
        {
            int framesDecoded = 0;
            while (framesDecoded != -1) {
                // The real AudioSource blocks on pMsgQ->pop()
                msleep(10);
                framesDecoded = processAudioMsg(pMsgQ, pStatusQ);
            }
        }

        virtual void testEqual(Bitmap& resultBmp, const std::string& sFName, 
                avg::PixelFormat pf = NO_PIXELFORMAT, float maxAverage=1.0,
                float maxStdDev=1.0)
        {
            GraphicsTest::testEqual(resultBmp, sFName, pf, maxAverage, maxStdDev);
        }

        void testEqual(Bitmap& resultBmp, Bitmap& BaselineBmp,
                const std::string& sFName, float maxAverage=1.0f, float maxStdDev=1.0f)
        {
            GraphicsTest::testEqual(resultBmp, BaselineBmp, sFName, 
                    maxAverage, maxStdDev);
        }

        string getMediaLoc(const string& sFilename)
        {
            return getSrcDirName()+"../test/media/"+sFilename;
        }

    private:
        string getDecoderName(bool bThreaded, bool bUseHardwareAcceleration)
        {
            string sName = "(";
            if (bThreaded) {
                sName += "Threaded";
            } else {
                sName += "Sync";
            }
            if (bUseHardwareAcceleration) {
                sName += ", VDPAU)";
            } else {
                sName += ")";
            }
            return sName;
        }

        bool m_bThreaded;
        bool m_bUseHardwareAcceleration;
};

class VideoDecoderTest: public DecoderTest {
    public:
        VideoDecoderTest(bool bThreaded, bool bUseHardwareAcceleration)
            : DecoderTest("VideoDecoderTest", bThreaded, bUseHardwareAcceleration)
        {}

        void runTests()
        {
            basicFileTest("mpeg1-48x48.mov", 30);
            basicFileTest("mjpeg-48x48.avi", 202);
            testSeeks("mjpeg-48x48.avi");
        }

    private:
        void basicFileTest(const string& sFilename, int expectedNumFrames) 
        {
            try {
                cerr << "    Testing " << sFilename << endl;

                VideoDecoderPtr pDecoder = createDecoder();
                pDecoder->open(getMediaLoc(sFilename), useHardwareAcceleration(), true);
                IntPoint frameSize = pDecoder->getSize();
                TEST(frameSize == IntPoint(48, 48));
                TEST(pDecoder->getVideoInfo().m_bHasVideo);
                TEST(pDecoder->getVideoInfo().m_Duration != 0);
                TEST(pDecoder->getStreamFPS() != 0);
                TEST(pDecoder->getFPS() != 0);
                pDecoder->startDecoding(false, getAudioParams());
                TEST(pDecoder->getPixelFormat() == B8G8R8X8);
                BitmapPtr pBmp(new Bitmap(frameSize, B8G8R8X8));

                // Test first two frames.
                pDecoder->renderToBmp(pBmp, -1);
                testEqual(*pBmp, sFilename+"_1", B8G8R8X8);
                TEST(pDecoder->getCurFrame() == 0);
                TEST(pDecoder->getCurTime() == 0);

                pDecoder->renderToBmp(pBmp, -1);
                testEqual(*pBmp, sFilename+"_2", B8G8R8X8);
                pDecoder->close();
                
                readWholeFile(sFilename, 1, expectedNumFrames); 
                readWholeFile(sFilename, 0.5, expectedNumFrames); 
                readWholeFile(sFilename, 2, expectedNumFrames/2); 
            } catch (Exception & ex) {
                cerr << string(m_IndentLevel+6, ' ') << ex.getStr() << endl;
                throw;
            }
        }

        void testSeeks(const string& sFilename)
        {
            cerr << "    Testing " << sFilename << " (seek)" << endl;

            VideoDecoderPtr pDecoder = createDecoder();
            pDecoder->open(getMediaLoc(sFilename), useHardwareAcceleration(), true);
            pDecoder->startDecoding(false, getAudioParams());

            // Seek forward
            testSeek(100, sFilename, pDecoder);
            // Seek backward
            testSeek(53, sFilename, pDecoder);
            // Seek to last frame
            testSeek(201, sFilename, pDecoder);

            pDecoder->close();
        }

        void testSeek(int frameNum, const string& sFilename, VideoDecoderPtr pDecoder)
        {
            IntPoint frameSize = pDecoder->getSize();

            BitmapPtr pBmp(new Bitmap(frameSize, B8G8R8X8));
            pDecoder->seek(float(frameNum)/pDecoder->getStreamFPS());
            pDecoder->renderToBmp(pBmp, -1);
            testEqual(*pBmp, sFilename+"_"+toString(frameNum), B8G8R8X8);

        }

        void readWholeFile(const string& sFilename, float speedFactor, 
                int expectedNumFrames)
        {
            // Read whole file, test last image.
            VideoDecoderPtr pDecoder = createDecoder();
            pDecoder->open(getMediaLoc(sFilename), useHardwareAcceleration(), true);
            IntPoint frameSize = pDecoder->getSize();
            float timePerFrame = (1.0f/pDecoder->getFPS())*speedFactor;
            pDecoder->startDecoding(false, getAudioParams());
            BitmapPtr pBmp(new Bitmap(frameSize, B8G8R8X8));
            int numFrames = 0;
            float curTime = 0;

            while (!pDecoder->isEOF()) {
                FrameAvailableCode frameAvailable = pDecoder->renderToBmp(pBmp, curTime);
                if (frameAvailable == FA_NEW_FRAME) {
/*                    
                    stringstream ss;
                    ss << "resultimages/" << sFilename << numFrames << ".png";
                    pBmp->save(ss.str());
*/                    
                    numFrames++;
                } else {
                    msleep(0);
                }
                if (frameAvailable == FA_NEW_FRAME || frameAvailable == FA_USE_LAST_FRAME)
                { 
                    curTime += timePerFrame;
                }
            }
//            cerr << "numFrames: " << numFrames << 
//                    ", expectedNumFrames: " << expectedNumFrames << endl;
            TEST(numFrames == expectedNumFrames);
            if (speedFactor == 1) {
                testEqual(*pBmp, sFilename+"_end", B8G8R8X8);
            }
            
            // Test loop.
            pDecoder->loop();
            pDecoder->renderToBmp(pBmp, -1);
            testEqual(*pBmp, sFilename+"_loop", B8G8R8X8);

            pDecoder->close();
        }

};

class AudioDecoderTest: public DecoderTest {
    public:
        AudioDecoderTest()
          : DecoderTest("AudioDecoderTest", true, true)
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
                    AsyncVideoDecoderPtr pDecoder = 
                            dynamic_pointer_cast<AsyncVideoDecoder>(createDecoder());
                    pDecoder->open(getMediaLoc(sFilename), useHardwareAcceleration(), true);
                    TEST(pDecoder->getVideoInfo().m_bHasAudio);
                    pDecoder->startDecoding(false, getAudioParams());
                    AudioMsgQueuePtr pMsgQ = pDecoder->getAudioMsgQ();
                    AudioMsgQueuePtr pStatusQ = pDecoder->getAudioStatusQ();
                    int totalFramesDecoded = 0;
                    readAudioToEOF(pDecoder, pMsgQ, pStatusQ, totalFramesDecoded, true);

                    // Check if we've decoded the whole file.
                    int framesInDuration = int(pDecoder->getVideoInfo().m_Duration*44100);
//                    cerr << "framesInDuration: " << framesInDuration << endl;
//                    cerr << "framesDecoded: " << totalFramesDecoded << endl;
                    TEST(abs(totalFramesDecoded-framesInDuration) < 65);
                }
                {
                    cerr << "      Seek test." << endl;
                    AsyncVideoDecoderPtr pDecoder = 
                            dynamic_pointer_cast<AsyncVideoDecoder>(createDecoder());
                    pDecoder->open(getMediaLoc(sFilename), useHardwareAcceleration(), true);
                    float duration = pDecoder->getVideoInfo().m_Duration;
                    pDecoder->startDecoding(false, getAudioParams());
                    AudioMsgQueuePtr pMsgQ = pDecoder->getAudioMsgQ();
                    AudioMsgQueuePtr pStatusQ = pDecoder->getAudioStatusQ();
                    pDecoder->seek(duration/2);
                    processAudioSeek(pMsgQ, pStatusQ);
                    int totalFramesDecoded = 0;

                    readAudioToEOF(pDecoder, pMsgQ, pStatusQ, totalFramesDecoded, false);
                    if (sFilename.find(".mp3") == string::npos) {
                        // Check if we've decoded half the file.
                        // TODO: Find out why there are problems with this
                        // for mp3 files.
                        int framesInDuration = 
                                int(pDecoder->getVideoInfo().m_Duration*44100);
//                        cerr << "framesDecoded: " << totalFramesDecoded << endl;
//                        cerr << "framesInDuration: " << framesInDuration << endl;
                        TEST(abs(totalFramesDecoded-framesInDuration/2) < 65);
                    }
                }

            } catch (Exception & ex) {
                cerr << string(m_IndentLevel+6, ' ') << ex.getStr() << endl;
                throw;
            }
        }

        void readAudioToEOF(AsyncVideoDecoderPtr pDecoder, AudioMsgQueuePtr pMsgQ, 
                AudioMsgQueuePtr pStatusQ, int& totalFramesDecoded,
                bool bCheckTimestamps) 
        {
            int numWrongTimestamps = 0;
            while (!pDecoder->isEOF()) {
                int framesDecoded = 0;
                while (framesDecoded == 0 && !pDecoder->isEOF()) {
                    framesDecoded = processAudioMsg(pMsgQ, pStatusQ);
                    AVG_ASSERT(framesDecoded != -1);
                    pDecoder->updateAudioStatus();
                    msleep(0);
                }
                totalFramesDecoded += framesDecoded;
                float curTime = float(totalFramesDecoded)/44100;
                if (abs(curTime-pDecoder->getCurTime()) > 0.02f) {
                    numWrongTimestamps++;
                }
            }
            if (bCheckTimestamps) {
                if (numWrongTimestamps>0) {
                    TEST_FAILED(numWrongTimestamps << " wrong timestamps.");
                }
            }
        }
};


class AVDecoderTest: public DecoderTest {
    public:
        AVDecoderTest(bool bUseHardwareAcceleration)
          : DecoderTest("AVDecoderTest", true, bUseHardwareAcceleration)
        {}

        void runTests()
        {
            basicFileTest("mpeg1-48x48-sound.avi", 30);
        }

    private:
        void basicFileTest(const string& sFilename, int expectedNumFrames)
        {
            VideoDecoderPtr pDecoder = createDecoder();
            pDecoder->open(getMediaLoc(sFilename), useHardwareAcceleration(), true);
            TEST(pDecoder->getVideoInfo().m_bHasVideo);
            TEST(pDecoder->getStreamFPS() != 0);
            pDecoder->startDecoding(false, getAudioParams());
            AudioMsgQueuePtr pMsgQ;
            AudioMsgQueuePtr pStatusQ;
            
            pMsgQ = dynamic_pointer_cast<AsyncVideoDecoder>(pDecoder) ->getAudioMsgQ();
            pStatusQ = dynamic_pointer_cast<AsyncVideoDecoder>(pDecoder)
                ->getAudioStatusQ();
            TEST(pDecoder->getVideoInfo().m_bHasAudio);
            
            IntPoint frameSize = pDecoder->getSize();
            BitmapPtr pBmp(new Bitmap(frameSize, B8G8R8X8));
            int numFrames = 0;
            int totalFramesDecoded = 0;
            float curTime = 0;
            
            while (!pDecoder->isEOF()) {
                FrameAvailableCode frameAvailable;
                do {
                    frameAvailable = pDecoder->renderToBmp(pBmp, curTime);
                    msleep(0);
                } while (frameAvailable == FA_STILL_DECODING);
                if (frameAvailable == FA_NEW_FRAME) {
//                    stringstream ss;
//                    ss << sFilename << numFrames << ".png";
//                    pBmp->save(ss.str());
                    numFrames++;
                }
                int framesDecoded = 0;
                while (framesDecoded == 0 && !pDecoder->isEOF()) {
                    framesDecoded = processAudioMsg(pMsgQ, pStatusQ);
                    dynamic_pointer_cast<AsyncVideoDecoder>(pDecoder)
                        ->updateAudioStatus();
                    msleep(0);
                }
                totalFramesDecoded += framesDecoded;

                curTime += 1.0f/pDecoder->getFPS();
            }
            TEST(pDecoder->isEOF());
            TEST(numFrames == expectedNumFrames);
            testEqual(*pBmp, sFilename+"_end", B8G8R8X8);

            // Test loop.
            pDecoder->seek(0);
            processAudioSeek(pMsgQ, pStatusQ);
            pDecoder->renderToBmp(pBmp, -1);
            testEqual(*pBmp, sFilename+"_loop", B8G8R8X8);

            pDecoder->close();
        }
};


class VideoTestSuite: public TestSuite {
public:
    VideoTestSuite() 
        : TestSuite("VideoTestSuite")
    {
        addAudioTests();
        addVideoTests(false);
#ifdef AVG_ENABLE_VDPAU
        if (VDPAUDecoder::isAvailable()) {
            addVideoTests(true);
        } else {
            cerr << "Skipping VDPAU tests: VDPAU configured but not available." << endl;
        }
#else
        cerr << "Skipping VDPAU tests: VDPAU not configured." << endl;
#endif
    }
private:

    void addAudioTests()
    {
        addTest(TestPtr(new AudioDecoderTest()));
    }

    void addVideoTests(bool bUseHardwareAcceleration)
    {
        addTest(TestPtr(new VideoDecoderTest(false, bUseHardwareAcceleration)));
        addTest(TestPtr(new VideoDecoderTest(true, bUseHardwareAcceleration)));

        addTest(TestPtr(new AVDecoderTest(bUseHardwareAcceleration)));
    }
};


int main(int nargs, char** args)
{
    ThreadProfiler* pProfiler = ThreadProfiler::get();
    pProfiler->setName("main");

    GraphicsTest::createResultImgDir();

    BitmapLoader::init(true);
    VideoTestSuite suite;
    bool bOk;
    
    suite.runTests();
    bOk = suite.isOk();
/*    
    while(true) {
        suite.runTests();
        bOk = suite.isOk();
        if (!bOk) {
            return 1;
        }
    }
*/    
    if (bOk) {
        return 0;
    } else {
        return 1;
    }
}

