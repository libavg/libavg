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

#ifndef _WorkerThread_H_
#define _WorkerThread_H_

#include "Command.h"
#include "Exception.h"
#include "Logger.h"
#include "Queue.h"
#include "Profiler.h"
#include "ThreadProfiler.h"

#include <boost/shared_ptr.hpp>

#include <iostream>

namespace avg {


template<class DERIVED_THREAD>
class WorkerThread {
public:
    typedef Queue<Command<DERIVED_THREAD> > CmdQueue;
    typedef boost::shared_ptr<CmdQueue> CmdQueuePtr;

    WorkerThread(const std::string& sName, CmdQueue& CmdQ);
    virtual ~WorkerThread() {};
    void operator()();

    void stop();

private:
    virtual bool init();
    virtual bool work() = 0;
    virtual void deinit() {};

    void processCommands();

    std::string m_sName;
    bool m_bShouldStop;
    CmdQueue& m_CmdQ;

    ThreadProfilerPtr m_pProfiler;
};

template<class DERIVED_THREAD>
WorkerThread<DERIVED_THREAD>::WorkerThread(const std::string& sName, CmdQueue& CmdQ)
    : m_sName(sName),
      m_bShouldStop(false),
      m_CmdQ(CmdQ)
{
}

template<class DERIVED_THREAD>
void WorkerThread<DERIVED_THREAD>::operator()()
{
    try {
        m_pProfiler = ThreadProfilerPtr(new ThreadProfiler(m_sName));
        Profiler::get().registerThreadProfiler(m_pProfiler);
        bool bOK;
        bOK = init();
        if (!bOK) {
            return;
        }
        m_pProfiler->start();
        while (!m_bShouldStop) {
            bOK = work();
            if (!bOK) {
                m_bShouldStop = true;
            } else {
                processCommands();
            }
            m_pProfiler->reset();
        }
        deinit();
    } catch (const Exception& e) {
         AVG_TRACE(Logger::ERROR, "Uncaught exception in thread " << m_sName << ": "
                  << e.GetStr());
    }
}
    
template<class DERIVED_THREAD>
bool WorkerThread<DERIVED_THREAD>::init()
{
    return true;
}

template<class DERIVED_THREAD>
void WorkerThread<DERIVED_THREAD>::processCommands()
{
    if (!m_CmdQ.empty()) {
        try {
            // This loop always ends in an exception when the Queue is empty.
            while (true) {
                Command<DERIVED_THREAD> Cmd = m_CmdQ.pop(false);
                Cmd.execute(dynamic_cast<DERIVED_THREAD*>(this));
            }
        } catch (const Exception& e) {
            if (e.GetCode() != AVG_ERR_QUEUE_EMPTY) {
                AVG_TRACE(Logger::ERROR, "Uncaught exception in thread " 
                        << m_sName << ": " << e.GetStr());
            }
        }
    }
   
}

template<class DERIVED_THREAD>
void WorkerThread<DERIVED_THREAD>::stop() {
    m_bShouldStop = true;
}

}

#endif
