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

#include "../base/Logger.h"
#include "../base/Profiler.h"
#include "../base/ScopeTimer.h"

#include <Magick++.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace avg;
using namespace std;

static ProfilingZone FillI8ProfilingZone("FilterFillI8");

void runFillI8PerformanceTest()
{
    BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(1024,1024), I8));
    ScopeTimer Timer(FillI8ProfilingZone);
    for (int i=0; i<1000; ++i) {
        FilterFill<Pixel8>(Pixel8(0)).applyInPlace(pBmp);
    }
}

static ProfilingZone FillRGBProfilingZone("FilterFillRGB");

void runFillRGBPerformanceTest()
{
    BitmapPtr pBmp = BitmapPtr(new Bitmap(IntPoint(1024,1024), R8G8B8));
    ScopeTimer Timer(FillRGBProfilingZone);
    for (int i=0; i<1000; ++i) {
        FilterFill<Pixel24>(Pixel24(0,0,0)).applyInPlace(pBmp);
    }
}

void runPerformanceTests()
{
    runFillI8PerformanceTest();
    runFillRGBPerformanceTest();
}

int main(int nargs, char** args)
{
    Logger::get()->setCategories(Logger::PROFILE);
    ThreadProfilerPtr pThreadProfiler = ThreadProfilerPtr(new ThreadProfiler("Main"));
    Profiler::get().registerThreadProfiler(pThreadProfiler);
    ThreadProfiler::get()->start();
    runPerformanceTests();
    ThreadProfiler::get()->reset();
    Profiler::get().dumpStatistics();
}

