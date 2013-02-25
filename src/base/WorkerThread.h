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

#ifndef _WorkerThread_H_
#define _WorkerThread_H_

#include "../api.h"
#include "Command.h"
#include "Exception.h"
#include "Logger.h"
#include "Queue.h"
#include "ThreadProfiler.h"
#include "CmdQueue.h"

#include <boost/shared_ptr.hpp>

#include <iostream>

namespace avg {

void setAffinityMask(bool bIsMainThread);

template<class DERIVED_THREAD>
class AVG_TEMPLATE_API WorkerThread {
public:
    typedef Command<DERIVED_THREAD> Cmd;
    typedef typename boost::shared_ptr<Cmd> CmdPtr;
    typedef CmdQueue<DERIVED_THREAD> CQueue;
    typedef typename boost::shared_ptr<CQueue> CQueuePtr;

    WorkerThread(const std::string& sName, CQueue& CmdQ,
            long logCategory=Logger::category::PROFILE);
    WorkerThread(WorkerThread const& other);
    virtual ~WorkerThread();
    void operator()();

    void waitForCommand();
    void stop();

private:
    virtual bool init();
    virtual bool work() = 0;
    virtual void deinit() {};

    void processCommands();

    std::string m_sName;
    bool m_bShouldStop;
    CQueue& m_CmdQ;
    long m_LogCategory;
};

template<class DERIVED_THREAD>
WorkerThread<DERIVED_THREAD>::WorkerThread(const std::string& sName, CQueue& CmdQ, 
        long logCategory)
    : m_sName(sName),
      m_bShouldStop(false),
      m_CmdQ(CmdQ),
      m_LogCategory(logCategory)
{
}

template<class DERIVED_THREAD>
WorkerThread<DERIVED_THREAD>::WorkerThread(WorkerThread const& other)
    : m_CmdQ(other.m_CmdQ)
{
    m_sName = other.m_sName;
    m_bShouldStop = other.m_bShouldStop;
    m_LogCategory = other.m_LogCategory;
}

template<class DERIVED_THREAD>
WorkerThread<DERIVED_THREAD>::~WorkerThread()
{
}

template<class DERIVED_THREAD>
void WorkerThread<DERIVED_THREAD>::operator()()
{
    try {
        setAffinityMask(false);
        ThreadProfiler* pProfiler = ThreadProfiler::get();
        pProfiler->setName(m_sName);
        pProfiler->setLogCategory(m_LogCategory);
        bool bOK;
        bOK = init();
        if (!bOK) {
            return;
        }
        pProfiler->start();
        while (!m_bShouldStop) {
            bOK = work();
            if (!bOK) {
                m_bShouldStop = true;
            } else {
                processCommands();
            }
        }
        deinit();
        pProfiler->dumpStatistics();
        pProfiler->kill();
    } catch (const Exception& e) {
         AVG_LOG_ERROR("Uncaught exception in thread " << m_sName << ": " << e.getStr());
         throw;
    }
}

template<class DERIVED_THREAD>
void WorkerThread<DERIVED_THREAD>::waitForCommand() 
{
    CmdPtr pCmd = m_CmdQ.pop(true);
    pCmd->execute(dynamic_cast<DERIVED_THREAD*>(this));
}

template<class DERIVED_THREAD>
void WorkerThread<DERIVED_THREAD>::stop() 
{
    m_bShouldStop = true;
}

template<class DERIVED_THREAD>
bool WorkerThread<DERIVED_THREAD>::init()
{
    return true;
}

template<class DERIVED_THREAD>
void WorkerThread<DERIVED_THREAD>::processCommands()
{
    CmdPtr pCmd = m_CmdQ.pop(false);
    while (pCmd) {
        pCmd->execute(dynamic_cast<DERIVED_THREAD*>(this));
        pCmd = m_CmdQ.pop(false);
    }
}

}

#endif
