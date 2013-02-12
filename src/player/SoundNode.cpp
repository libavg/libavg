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
#include "SoundNode.h"
#include "Player.h"
#include "TypeDefinition.h"
#include "Canvas.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/ObjectCounter.h"

#include "../audio/AudioEngine.h"

#include "../video/AsyncVideoDecoder.h"

#include <iostream>
#include <sstream>

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace boost;
using namespace std;

namespace avg {

void SoundNode::registerType()
{
    TypeDefinition def = TypeDefinition("sound", "areanode", 
            ExportedObject::buildObject<SoundNode>)
        .addArg(Arg<UTF8String>("href", "", false, offsetof(SoundNode, m_href)))
        .addArg(Arg<bool>("loop", false, false, offsetof(SoundNode, m_bLoop)))
        .addArg(Arg<float>("volume", 1.0, false, offsetof(SoundNode, m_Volume)))
        ;
    TypeRegistry::get()->registerType(def);
}

SoundNode::SoundNode(const ArgList& args)
    : m_Filename(""),
      m_pEOFCallback(0),
      m_SeekBeforeCanRenderTime(0),
      m_pDecoder(0),
      m_Volume(1.0),
      m_State(Unloaded),
      m_AudioID(-1)
{
    args.setMembers(this);
    m_Filename = m_href;
    initFilename(m_Filename);
    m_pDecoder = new AsyncVideoDecoder(8);

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
    return (long long)(m_pDecoder->getVideoInfo().m_Duration*1000);
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
    return (long long)(m_pDecoder->getCurTime()*1000);
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
    if (pEOFCallback == Py_None) {
        m_pEOFCallback = 0;
    } else {
        avgDeprecationWarning("1.8", "SoundNode.setEOFCallback()", 
                "Node.subscribe(END_OF_FILE)");
        Py_INCREF(pEOFCallback);
        m_pEOFCallback = pEOFCallback;
    }
}

void SoundNode::connectDisplay()
{
    if (!AudioEngine::get()) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Sound nodes can only be created if audio is not disabled."); 
    }
    checkReload();
    AreaNode::connectDisplay();
    long long curTime = Player::get()->getFrameTime(); 
    if (m_State != Unloaded) {
        startDecoding();
        m_StartTime = curTime;
        m_PauseTime = 0;
    }
    if (m_State == Paused) {
        m_PauseStartTime = curTime;
    } 
}

void SoundNode::connect(CanvasPtr pCanvas)
{
    checkReload();
    AreaNode::connect(pCanvas);
    pCanvas->registerFrameEndListener(this);
}

void SoundNode::disconnect(bool bKill)
{
    changeSoundState(Unloaded);
    getCanvas()->unregisterFrameEndListener(this);
    if (bKill) {
        setEOFCallback(Py_None);
    }
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

float SoundNode::getVolume()
{
    return m_Volume;
}

void SoundNode::setVolume(float volume)
{
    if (volume < 0) {
        volume = 0;
    }
    m_Volume = volume;
    if (m_AudioID != -1) {
        AudioEngine::get()->setSourceVolume(m_AudioID, volume);
    }
}

void SoundNode::checkReload()
{
    string fileName (m_href);
    if (m_href != "") {
        initFilename(fileName);
        if (fileName != m_Filename && m_State != Unloaded) {
            changeSoundState(Unloaded);
            m_Filename = fileName;
            changeSoundState(Paused);
        } else {
            m_Filename = fileName;
        }
    } else {
        changeSoundState(Unloaded);
        m_Filename = "";
    }
}

void SoundNode::onFrameEnd()
{
    if (m_State == Playing) {
        m_pDecoder->updateAudioStatus();
    }
    if (m_State == Playing && m_pDecoder->isEOF()) {
        NodePtr pTempThis = getSharedThis();
        onEOF();
    }
}

void SoundNode::changeSoundState(SoundState newSoundState)
{
    if (newSoundState == m_State) {
        return;
    }
    if (m_State == Unloaded) {
        open();
    }
    if (newSoundState == Unloaded) {
        close();
    }
    if (getState() == NS_CANRENDER) {
        long long curTime = Player::get()->getFrameTime(); 
        if (m_State == Unloaded) {
            startDecoding();
            m_StartTime = curTime;
            m_PauseTime = 0;
        }
        if (newSoundState == Paused) {
            m_PauseStartTime = curTime;
            AudioEngine::get()->pauseSource(m_AudioID);
        } else if (newSoundState == Playing && m_State == Paused) {
            m_PauseTime += curTime-m_PauseStartTime;
            AudioEngine::get()->playSource(m_AudioID);
        }
    }
    m_State = newSoundState;
}

void SoundNode::seek(long long destTime) 
{
    if (getState() == NS_CANRENDER) {    
        AudioEngine::get()->notifySeek(m_AudioID);
        m_pDecoder->seek(float(destTime)/1000);
        m_StartTime = Player::get()->getFrameTime() - destTime;
        m_PauseTime = 0;
        m_PauseStartTime = Player::get()->getFrameTime();
    } else {
        // If we get a seek command before decoding has really started, we need to defer 
        // the actual seek until the decoder is ready.
        m_SeekBeforeCanRenderTime = destTime;
    }
}

void SoundNode::open()
{
    m_pDecoder->open(m_Filename, false, true);
    VideoInfo videoInfo = m_pDecoder->getVideoInfo();
    if (!videoInfo.m_bHasAudio) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("SoundNode: Opening "+m_Filename
                        +" failed. No audio stream found."));
    }
}

void SoundNode::startDecoding()
{
    AudioEngine* pEngine = AudioEngine::get();
    m_pDecoder->startDecoding(false, pEngine->getParams());
    m_AudioID = pEngine->addSource(*m_pDecoder->getAudioMsgQ(), 
            *m_pDecoder->getAudioStatusQ());
    pEngine->setSourceVolume(m_AudioID, m_Volume);
    if (m_SeekBeforeCanRenderTime != 0) {
        seek(m_SeekBeforeCanRenderTime);
        m_SeekBeforeCanRenderTime = 0;
    }
}

void SoundNode::close()
{
    if (m_AudioID != -1) {
        AudioEngine::get()->removeSource(m_AudioID);
        m_AudioID = -1;
    }
    m_pDecoder->close();
}

void SoundNode::exceptionIfUnloaded(const std::string& sFuncName) const
{
    if (m_State == Unloaded) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("SoundNode.")+sFuncName+" failed: sound not loaded.");
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
            throw py::error_already_set();
        }
        Py_DECREF(result);
    }
    notifySubscribers("END_OF_FILE");
}

}
