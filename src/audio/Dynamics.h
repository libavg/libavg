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
//  Original author of this file is Andreas Beisler.
//


#pragma once
#ifndef __Dynamics_H__
#define __Dynamics_H__

#include <math.h>
#include <cmath>
#include <limits>
#include <memory.h>

#include "Processor.h"

#define LOOKAHEAD 64
#define AVG1 27
#define AVG2 38

// Dynamics processor (compressor & limiter).
template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
class Dynamics: public Processor<T, IN_CHANNELS, OUT_CHANNELS>
{
    public:
        Dynamics(T fs);
        virtual ~Dynamics();
        void Process(T* inSamples, T* outSamples);

        void setThreshold(T threshold);
        T getThreshold() const;
        void setRmsTime(T rmsTime);
        T getRmsTime() const;
        void setRatio(T ratio);
        T getRatio() const;
        void setAttackTime(T attTime);
        T getAttackTime() const;
        void setReleaseTime(T relTime);
        T getReleaseTime() const;
        void setMakeupGain(T makeupGain);
        T getMakeupGain() const;

    protected:
        void maxFilter(T& rms);

        T threshold_;
        T preGain_;

        T rmsTime_;
        T rmsCoef_;
        T rms1_;

        T* lookaheadBuf_;
        int lookaheadBufIdx_;

        T ratio_;
        T inverseRatio_;

        T attTime_;
        T attCoef_;
        T relTime_;
        T relCoef_;
        T env1_;

        T* avg1Buf_;
        int avg1BufRIdx_;
        int avg1BufWIdx_;
        T avg1Old_;

        T* avg2Buf_;
        int avg2BufRIdx_;
        int avg2BufWIdx_;
        T avg2Old_;

        T* delayBuf_;
        int delayBufIdx_;

        T makeupGain_;
        T postGain_;
};

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::Dynamics(T fs):
    Processor<T, IN_CHANNELS, OUT_CHANNELS>(fs),
    threshold_(0.),
    preGain_(1.),
    rmsTime_(0.),
    rmsCoef_(0.),
    rms1_(0.),
    lookaheadBuf_(0),
    lookaheadBufIdx_(0),
    ratio_(std::numeric_limits<T>::infinity()),
    inverseRatio_(0.),
    attTime_(0.),
    attCoef_(0.),
    relTime_(0.),
    relCoef_(0.),
    env1_(0.),
    avg1Buf_(0),
    avg1BufRIdx_(0),
    avg1BufWIdx_(AVG1 - 1),
    avg1Old_(0.),
    avg2Buf_(0),
    avg2BufRIdx_(0),
    avg2BufWIdx_(AVG2 - 1),
    avg2Old_(0.),
    delayBuf_(0),
    delayBufIdx_(0),
    makeupGain_(0.),
    postGain_(1.)
{
    lookaheadBuf_ = new T[LOOKAHEAD];
    for (int i = 0; i < LOOKAHEAD; i++) {
        lookaheadBuf_[i] = 1.f;
    }

    avg1Buf_ = new T[AVG1];
    memset(avg1Buf_, 0, sizeof(T) * (AVG1));

    avg2Buf_ = new T[AVG2];
    memset(avg2Buf_, 0, sizeof(T) * (AVG2));

    delayBuf_ = new T[LOOKAHEAD*OUT_CHANNELS];
    memset(delayBuf_, 0, sizeof(T)*LOOKAHEAD*OUT_CHANNELS);

    setThreshold(0.);
    setRmsTime(0.);
    setRatio(std::numeric_limits<T>::infinity());
    setAttackTime(0.);
    setReleaseTime(0.05);
    setMakeupGain(0.);
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::~Dynamics()
{
    delete[] lookaheadBuf_;

    delete[] avg1Buf_;
    delete[] avg2Buf_;

    delete[] delayBuf_;
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
void Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::maxFilter(T& rms)
{
    int j = lookaheadBufIdx_;
    for (int i = 0; i < LOOKAHEAD; i++)
    {
        j = (j+1)&(LOOKAHEAD-1);
        if (lookaheadBuf_[j] < rms) {
            lookaheadBuf_[j] = rms;
        }
    }
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
void Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::Process(T* inSamples, T* outSamples)
{
    Processor<T, IN_CHANNELS, OUT_CHANNELS>::Process(inSamples, outSamples);

    //---------------- Preprocessing
    T x = 0.f;
    for (int i = 0; i < OUT_CHANNELS; i++) {
        // Apply pregain
        const T tmp = outSamples[i] * preGain_;

        T abs = std::fabs(tmp);
        if (abs > x) {
            x = abs;
        }
    }

    //---------------- RMS
    T rms = (1. - rmsCoef_) * x * x + rmsCoef_ * rms1_;
    rms1_ = rms;
    rms   = sqrt(rms);

    //---------------- Max filter
    if (rms > 1.) {
        maxFilter(rms);
    }

    //---------------- Ratio
    T dbMax  = std::log10(lookaheadBuf_[lookaheadBufIdx_]);
    T dbComp = dbMax * inverseRatio_;
    T comp   = std::pow(static_cast<T>(10.), dbComp);
    T c      = comp / lookaheadBuf_[lookaheadBufIdx_];

    lookaheadBuf_[lookaheadBufIdx_] = 1.;
    lookaheadBufIdx_ = (lookaheadBufIdx_+1)%LOOKAHEAD;

    //---------------- Attack/release envelope
    if (env1_ <= c) {
        c = c + (env1_ - c) * relCoef_;
    } else {
        c = c + (env1_ - c) * attCoef_;
    }
    env1_ = c;

    //---------------- Smoothing
    const T tmp1           = avg1Old_ + c - avg1Buf_[avg1BufRIdx_];
    avg1Old_               = tmp1;
    avg1Buf_[avg1BufWIdx_] = c;
    c = tmp1;
    avg1BufRIdx_ = (avg1BufRIdx_+1)%AVG1;
    avg1BufWIdx_ = (avg1BufWIdx_+1)%AVG1;

    const T tmp2           = avg2Old_ + c - avg2Buf_[avg2BufRIdx_];
    avg2Old_               = tmp2;
    avg2Buf_[avg2BufWIdx_] = c;
    c = tmp2;
    avg2BufRIdx_ = (avg2BufRIdx_+1)%AVG2;
    avg2BufWIdx_ = (avg2BufWIdx_+1)%AVG2;

    c = c / (static_cast<T>(AVG1) * static_cast<T>(AVG2));

    //---------------- Postprocessing
    for (int i = 0; i < OUT_CHANNELS; i++) {
        // Delay input samples
        const T in                 = delayBuf_[delayBufIdx_*OUT_CHANNELS+i];
        delayBuf_[delayBufIdx_*OUT_CHANNELS+i] = outSamples[i];

        // Apply control signal
        outSamples[i] = in * c * postGain_;
    }

    delayBufIdx_ = (delayBufIdx_+1)&(LOOKAHEAD-1);
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
void Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::setThreshold(T threshold)
{
    threshold_ = threshold;
    preGain_   = std::pow(10., -threshold / 20.);
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
T Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::getThreshold() const
{
    return threshold_;
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
void Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::setRmsTime(T rmsTime)
{
    rmsTime_ = rmsTime;
    rmsCoef_ = 0.;
    if (rmsTime > 0.) {
        rmsCoef_ = std::pow(0.001, 1. / (Processor<T, IN_CHANNELS, OUT_CHANNELS>::m_fs * rmsTime));
    }
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
T Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::getRmsTime() const
{
    return rmsTime_;
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
void Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::setRatio(T ratio)
{
    ratio_        = ratio;
    inverseRatio_ = 1. / ratio;
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
T Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::getRatio() const
{
    return ratio_;
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
void Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::setAttackTime(T attTime)
{
    attTime_ = attTime;
    attCoef_ = 0.;
    if (attTime > 0.) {
        attCoef_ = powf(0.001, 1. / (Processor<T, IN_CHANNELS, OUT_CHANNELS>::m_fs * attTime));
    }
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
T Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::getAttackTime() const
{
    return attTime_;
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
void Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::setReleaseTime(T relTime)
{
    relTime_ = relTime;
    relCoef_ = 0.;
    if (relTime > 0.) {
        relCoef_ = powf(0.001, 1. / (Processor<T, IN_CHANNELS, OUT_CHANNELS>::m_fs * relTime));
    }
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
T Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::getReleaseTime() const
{
    return relTime_;
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
void Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::setMakeupGain(T makeupGain)
{
    makeupGain_ = makeupGain;
    postGain_   = std::pow(10., makeupGain / 20.);
}

template<typename T, int IN_CHANNELS, int OUT_CHANNELS>
T Dynamics<T, IN_CHANNELS, OUT_CHANNELS>::getMakeupGain() const
{
    return makeupGain_;
}

#endif

