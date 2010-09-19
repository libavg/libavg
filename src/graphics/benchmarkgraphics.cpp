//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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


#include "Bitmap.h"
#include "Pixel32.h"
#include "Pixel24.h"
#include "Pixel16.h"
#include "Filtergrayscale.h"
#include "Filterfill.h"
#include "Filterflip.h"
#include "Filterfliprgb.h"
#include "Filterflipuv.h"
#include "Filter3x3.h"
#include "FilterConvol.h"
#include "HistoryPreProcessor.h"
#include "FilterHighpass.h"
#include "FilterGauss.h"
#include "FilterBlur.h"
#include "FilterBandpass.h"

#include "../base/TimeSource.h"

#include <Magick++.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace avg;
using namespace std;

#define NUM_RUNS 50

template<class TEST>
void runPerformanceTest()
{
    TEST PerfTest;
    long long StartTime = TimeSource::get()->getCurrentMicrosecs();
    for (int i=0; i<NUM_RUNS; ++i) {
        PerfTest.run();
    }
    double ActiveTime = (TimeSource::get()->getCurrentMicrosecs()-StartTime)/1000.; 
    cerr << PerfTest.getName() << ": " << ActiveTime/NUM_RUNS << " ms" << endl;
    
}

class PerfTestBase {
public:
    PerfTestBase(string sName) 
        : m_sName(sName)
    {
    }

    std::string getName()
    {
        return m_sName;
    }

private:
    std::string m_sName;
};

class FillI8PerfTest: public PerfTestBase {
public:
    FillI8PerfTest() 
        : PerfTestBase("FillI8PerfTest")
    {
        m_pBmp = BitmapPtr(new Bitmap(IntPoint(1024,1024), I8));
    }

    void run() {
        FilterFill<Pixel8> Filter = FilterFill<Pixel8>(Pixel8(0));
        Filter.applyInPlace(m_pBmp);
    }

private:
    BitmapPtr m_pBmp;
};

class FillRGBPerfTest: public PerfTestBase {
public:
    FillRGBPerfTest() 
        : PerfTestBase("FillRGBPerfTest")
    {
        m_pBmp = BitmapPtr(new Bitmap(IntPoint(1024,1024), R8G8B8));
    }

    void run() {
        FilterFill<Pixel24> Filter = FilterFill<Pixel24>(Pixel24(0,0,0));
        Filter.applyInPlace(m_pBmp);
    }

private:
    BitmapPtr m_pBmp;
};

class EqualityI8PerfTest: public PerfTestBase {
public:
    EqualityI8PerfTest() 
        : PerfTestBase("EqualityI8PerfTest")
    {
        m_pBmp = BitmapPtr(new Bitmap(IntPoint(1024,1024), I8));
    }

    void run() {
        Bitmap CopyBmp = *m_pBmp;
    }

private:
    BitmapPtr m_pBmp;
};

class CopyI8PerfTest: public PerfTestBase {
public:
    CopyI8PerfTest() 
        : PerfTestBase("CopyI8PerfTest")
    {
        m_pBmp = BitmapPtr(new Bitmap(IntPoint(1024,1024), I8));
    }

    void run() {
        Bitmap CopyBmp (*m_pBmp);
    }

private:
    BitmapPtr m_pBmp;
};

class CopyRGBPerfTest: public PerfTestBase {
public:
    CopyRGBPerfTest() 
        : PerfTestBase("CopyRGBPerfTest")
    {
        m_pBmp = BitmapPtr(new Bitmap(IntPoint(1024,1024), R8G8B8));
    }

    void run() {
        Bitmap CopyBmp (*m_pBmp);
    }

private:
    BitmapPtr m_pBmp;
};

class YUV2RGBPerfTest: public PerfTestBase {
public:
    YUV2RGBPerfTest() 
        : PerfTestBase("YUV2RGBPerfTest")
    {
        m_pYBmp = BitmapPtr(new Bitmap(IntPoint(1024, 1024), I8));
        m_pUBmp = BitmapPtr(new Bitmap(IntPoint(512, 512), I8));
        m_pVBmp = BitmapPtr(new Bitmap(IntPoint(512, 512), I8));
    }

    void run() {
        Bitmap RGBBmp(IntPoint(1024, 1024), B8G8R8X8);
        RGBBmp.copyYUVPixels(*m_pYBmp, *m_pUBmp, *m_pVBmp, false);
    }

private:
    BitmapPtr m_pYBmp;
    BitmapPtr m_pUBmp;
    BitmapPtr m_pVBmp;
        
};

void runPerformanceTests()
{
    runPerformanceTest<FillI8PerfTest>();
    runPerformanceTest<FillRGBPerfTest>();
    runPerformanceTest<EqualityI8PerfTest>();
    runPerformanceTest<CopyI8PerfTest>();
    runPerformanceTest<CopyRGBPerfTest>();
    runPerformanceTest<YUV2RGBPerfTest>();
}

int main(int nargs, char** args)
{
    runPerformanceTests();
}

