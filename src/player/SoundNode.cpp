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
#include "SoundNode.h"
#include "Player.h"
#include "NodeDefinition.h"
#include "Scene.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../audio/AudioEngine.h"

#include "../video/AsyncVideoDecoder.h"
#include "../video/FFMpegDecoder.h"

#include <iostream>
#include <sstream>

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace boost::python;
using namespace std;

namespace avg {

NodeDefinition SoundNode::createDefinition()
{
    return NodeDefinition("sound", VisibleNode::buildNode<SoundNode>)
        .extendDefinition(AreaNode::createDefinition())
        .addArg(Arg<UTF8String>("href", "", false, offsetof(SoundNode, m_href)))
        .addArg(Arg<bool>("loop", false, false, offsetof(SoundNode, m_bLoop)))
        .addArg(Arg<double>("volume", 1.0, false, offsetof(SoundNode, m_Volume)))
        ;
}

SoundNode::SoundNode(const ArgList& Args)
    : m_Filename(""),
      m_pEOFCallback(0),
      m_pDecoder(0),
      m_Volume(1.0),
      m_State(Unloaded)
{
    Args.setMembers(this);
    m_Filename = m_href;
    initFilename(m_Filename);
    VideoDecoderPtr pSyncDecoder(new FFMpegDecoder());
    m_pDecoder = new AsyncVideoDecoder(pSyncDecoder);

    ObjectCounter::get()->incRef(&typeid(*this));
}

SoundNode::~SoundNode()
{
    if (m_pDecoder) {
        delete m_pDecoder;
        m_pDecoder = 0;
    }
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

long long SoundNode::getDuration() const
{
    exceptionIfUnloaded("getDuration");
    return m_pDecoder->getVideoInfo().m_Duration;
}

std::string SoundNode::getAudioCodec() const
{
    exceptionIfUnloaded("getAudioCodec");
    return m_pDecoder->getVideoInfo().m_sACodec;
}

int SoundNode::getAudioSampleRate() const
{
    exceptionIfUnloaded("getAudioSampleRate");
    return m_pDecoder->getVideoInfo().m_SampleRate;
}

int SoundNode::getNumAudioChannels() const
{
    exceptionIfUnloaded("getNumAudioChannels");
    return m_pDecoder->getVideoInfo().m_NumAudioChannels;
}

long long SoundNode::getCurTime() const
{
    exceptionIfUnloaded("getCurTime");
    return m_pDecoder->getCurTime();
}

void SoundNode::seekToTime(long long Time)
{
    exceptionIfUnloaded("seekToTime");
    seek(Time);
}

bool SoundNode::getLoop() const
{
    return m_bLoop;
}

void SoundNode::setEOFCallback(PyObject * pEOFCallback)
{
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
    Py_INCREF(pEOFCallback);
    m_pEOFCallback = pEOFCallback;
}

void SoundNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    if (!pAudioEngine) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Sound nodes can only be created if audio is not disabled."); 
    }
    checkReload();
    AreaNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    long long CurTime = Player::get()->getFrameTime(); 
    if (m_State != Unloaded) {
        startDecoding();
        m_StartTime = CurTime;
        m_PauseTime = 0;
    }
    if (m_State == Paused) {
        m_PauseStartTime = CurTime;
    } 
}

void SoundNode::connect(ScenePtr pScene)
{
    AreaNode::connect(pScene);
    pScene->registerFrameEndListener(this);
}

void SoundNode::disconnect(bool bKill)
{
    changeSoundState(Unloaded);
    getScene()->unregisterFrameEndListener(this);
    AreaNode::disconnect(bKill);
}

void SoundNode::play()
{
    changeSoundState(Playing);
}

void SoundNode::stop()
{
    changeSoundState(Unloaded);
}

void SoundNode::pause()
{
    changeSoundState(Paused);
}

const UTF8String& SoundNode::getHRef() const
{
    return m_href;
}

void SoundNode::setHRef(const UTF8String& href)
{
    m_href = href;
    checkReload();
}

double SoundNode::getVolume()
{
    return m_Volume;
}

void SoundNode::setVolume(double Volume)
{
    if (Volume < 0) {
        Volume = 0;
    }
    m_Volume = Volume;
    if (m_pDecoder) {
        m_pDecoder->setVolume(Volume);
    }
}

void SoundNode::checkReload()
{
    string fileName (m_href);
    if (m_href != "") {
        initFilename(fileName);
        if (fileName != m_Filename) {
            SoundState oldState = m_State;
            changeSoundState(Unloaded);
            m_Filename = fileName;
            if (oldState != Unloaded) {
                changeSoundState(Paused);
            }
        }
    } else {
        changeSoundState(Unloaded);
        m_Filename = "";
    }
}

void SoundNode::onFrameEnd()
{
    if (m_State == Playing && m_pDecoder->isEOF(SS_AUDIO)) {
        onEOF();
    }
}

int SoundNode::fillAudioBuffer(AudioBufferPtr pBuffer)
{
    if (m_State == Playing) {
        return m_pDecoder->fillAudioBuffer(pBuffer);
    } else {
        return 0;
    }
}

void SoundNode::changeSoundState(SoundState NewSoundState)
{
    if (NewSoundState == m_State) {
        return;
    }
    if (m_State == Unloaded) {
        open();
    }
    if (NewSoundState == Unloaded) {
        close();
    }
    if (getState() == NS_CANRENDER) {
        long long CurTime = Player::get()->getFrameTime(); 
        if (m_State == Unloaded) {
            startDecoding();
            m_StartTime = CurTime;
            m_PauseTime = 0;
        }
        if (NewSoundState == Paused) {
            m_PauseStartTime = CurTime;
        } else if (NewSoundState == Playing && m_State == Paused) {
            m_PauseTime += CurTime-m_PauseStartTime;
        }
    }
    m_State = NewSoundState;
}

void SoundNode::seek(long long DestTime) 
{
    m_pDecoder->seek(DestTime);
    m_StartTime = Player::get()->getFrameTime() - DestTime;
    m_PauseTime = 0;
    m_PauseStartTime = Player::get()->getFrameTime();
}

void SoundNode::open()
{
    m_pDecoder->open(m_Filename, true);
    m_pDecoder->setVolume(m_Volume);
    VideoInfo videoInfo = m_pDecoder->getVideoInfo();
    if (!videoInfo.m_bHasAudio) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("SoundNode: Opening "+m_Filename
                        +" failed. No audio stream found."));
    }
}

void SoundNode::startDecoding()
{
    m_pDecoder->startDecoding(false, getAudioEngine()->getParams());
    if (getAudioEngine()) {
        getAudioEngine()->addSource(this);
    }
}

void SoundNode::close()
{
    if (getAudioEngine()) {
        getAudioEngine()->removeSource(this);
    }
    m_pDecoder->close();
}

void SoundNode::exceptionIfUnloaded(const std::string& sFuncName) const
{
    if (m_State == Unloaded) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("SoundNode.")+sFuncName+" failed: video not loaded.");
    }
}

void SoundNode::onEOF()
{
    seek(0);
    if (!m_bLoop) {
        changeSoundState(Paused);
    }
    if (m_pEOFCallback) {
        PyObject * arglist = Py_BuildValue("()");
        PyObject * result = PyEval_CallObject(m_pEOFCallback, arglist);
        Py_DECREF(arglist);
        if (!result) {
            throw error_already_set();
        }
        Py_DECREF(result);
    }
}

}
