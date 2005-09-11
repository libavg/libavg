//
// $Id$
//

#include "ParPort.h"

#include "../base/Logger.h"
#include "../player/MathHelper.h"
#include "../base/Exception.h"

#include <linux/ppdev.h>
#include <linux/parport.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <iostream>
#include <sstream>

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
}

bool ParPort::setDataLines(unsigned char theData)
{
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
}

bool ParPort::clearDataLines(unsigned char theData)
{
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
}

bool ParPort::setAllDataLines(unsigned char theData)
{
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
}    

bool ParPort::isAvailable() 
{
    return (_myFileDescriptor != -1);
}

bool ParPort::frob(int theLines, int theStatus) {
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
    if (_isOpen) {
        if (ioctl(_myFileDescriptor, PPWCONTROL, & theStatus) == -1) {
            AVG_TRACE(Logger::ERROR,
                    "ERROR: Failed to write control register: " 
                    << strerror(errno));
            return false;
        }
        return true;
    }
    return false;
}

}

