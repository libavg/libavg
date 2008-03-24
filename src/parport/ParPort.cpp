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

#include "ParPort.h"

#include "../avgconfigwrapper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/MathHelper.h"

#ifdef AVG_ENABLE_PARPORT 
#include <linux/ppdev.h>
#include <linux/parport.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <cerrno>

using namespace std;

namespace avg {

ParPort::ParPort()
    : _myFileDescriptor(-1),
      _myDeviceName(""),
      _isOpen(false),
      _myDataLines(0)
{
}

ParPort::~ParPort() {
    deinit();
}

void 
ParPort::init(const string& theDevice) {
    string myDevice = theDevice;
#ifdef AVG_ENABLE_PARPORT
    if (myDevice.empty()) {
        myDevice = "/dev/parport0";
    }
    _myFileDescriptor = open(myDevice.c_str(), O_RDONLY);
    if (_myFileDescriptor == -1) {
        AVG_TRACE(Logger::ERROR, 
                "Failed to open parallel port '" 
                 << myDevice << "': " << strerror(errno));
        return;
    } else {
        AVG_TRACE(Logger::CONFIG, "Parallel port opened.");
    }

    if (ioctl(_myFileDescriptor, PPCLAIM) == -1) {
        AVG_TRACE(Logger::ERROR,
                "Failed to claim parallel port: "
                 << strerror(errno));
        _myFileDescriptor = -1;
        return;
    }
    _isOpen = true;
#else
    AVG_TRACE(Logger::ERROR,
            "Failed to open parallel port. Support not compiled in.");
    _myFileDescriptor = -1;
#endif
    _myDeviceName = myDevice;
}

bool ParPort::setControlLine(int theLine, bool theValue) {
    int theStatus;
    if (theValue) {
        theStatus = theLine;
    } else {
        theStatus = 0;
    }
    return frob(theLine, theStatus); 
}

bool ParPort::getStatusLine(int theLine) {
#ifdef AVG_ENABLE_PARPORT
    if (_myFileDescriptor == -1) {
        return false;
    }
    unsigned char myStatus;
    int myOk = ioctl(_myFileDescriptor, PPRSTATUS, &myStatus);
    if (myOk == -1) {
        AVG_TRACE(Logger::ERROR,
                "Could not get parallel port status.");
        return false;
    }
    return (myStatus & theLine) == theLine;
#else
    return false;
#endif
}

bool ParPort::setDataLines(unsigned char theData)
{
#ifdef AVG_ENABLE_PARPORT
    if (_myFileDescriptor == -1) {
        return false;
    }
    _myDataLines |= theData;
    int myOk = ioctl(_myFileDescriptor, PPWDATA, &_myDataLines);
    if (myOk == -1) {
        AVG_TRACE(Logger::ERROR,
                "Could not write parallel port data.");
        return false;
    }    
    return true;
#else
    return false;
#endif
}

bool ParPort::clearDataLines(unsigned char theData)
{
#ifdef AVG_ENABLE_PARPORT
    if (_myFileDescriptor == -1) {
        return false;
    }
    _myDataLines &= ~theData;
    int myOk = ioctl(_myFileDescriptor, PPWDATA, &_myDataLines);
    if (myOk == -1) {
        AVG_TRACE(Logger::ERROR,
                "Could not write parallel port data.");
        return false;
    }    
    return true;
#else
    return false;
#endif
}

bool ParPort::setAllDataLines(unsigned char theData)
{
#ifdef AVG_ENABLE_PARPORT
    if (_myFileDescriptor == -1) {
        return false;
    }
    _myDataLines = theData;
    int myOk = ioctl(_myFileDescriptor, PPWDATA, &_myDataLines);
    if (myOk == -1) {
        AVG_TRACE(Logger::ERROR,
                "Could not write parallel port data.");
        return false;
    }    
    return true;
#else
    return false;
#endif
}    

bool ParPort::isAvailable() 
{
    return (_myFileDescriptor != -1);
}

bool ParPort::frob(int theLines, int theStatus) 
{
#ifdef AVG_ENABLE_PARPORT
    if (_myFileDescriptor == -1) {
        return false;
    }
    ppdev_frob_struct myFrob;
    myFrob.mask = theLines;
    myFrob.val = theStatus;
    int myOk = ioctl(_myFileDescriptor, PPFCONTROL, &myFrob);
    if (myOk == -1) {
        AVG_TRACE(Logger::ERROR,
                "Could not set parallel port control line.");
        return false;
    }
    return true;
#else
    return false;
#endif
}

void ParPort::deinit() {
    if (_isOpen) {
        if (::close(_myFileDescriptor) == -1) {
            AVG_TRACE(Logger::ERROR,
                    "Can't close parallel port '"
                    << _myDeviceName << "':" << strerror(errno));
        } else {
            AVG_TRACE(Logger::CONFIG, "Parallel port closed.");
        }
    }
}

bool
ParPort::writeControlRegister(unsigned char theStatus) {
#ifdef AVG_ENABLE_PARPORT
    if (_isOpen) {
        if (ioctl(_myFileDescriptor, PPWCONTROL, & theStatus) == -1) {
            AVG_TRACE(Logger::ERROR,
                    "ERROR: Failed to write control register: " 
                    << strerror(errno));
            return false;
        }
        return true;
    }
#endif
    return false;
}

}

