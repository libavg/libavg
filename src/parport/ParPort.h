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

#ifndef _ParPort_H_
#define _ParPort_H_

#include <string>

namespace avg {
    
enum ParPortData {
    BIT0 = 1,
    BIT1 = 2,
    BIT2 = 4,
    BIT3 = 8,
    BIT4 = 16,
    BIT5 = 32,
    BIT6 = 64,
    BIT7 = 128
};
        
class ParPort
{
    public:
        ParPort();
        virtual ~ParPort();
        void init(const std::string& theDevice);
        bool setControlLine(int theLine, bool theValue);
        bool getStatusLine(int theLine);
        bool setDataLines(unsigned char theData);
        bool clearDataLines(unsigned char theData);
        bool setAllDataLines(unsigned char theData);
        bool isAvailable();

    private:
        bool frob(int theLines, int theStatus);
        void deinit();
        bool writeControlRegister(unsigned char theStatus);
        
        int         _myFileDescriptor;
        std::string _myDeviceName;
        bool        _isOpen;

        unsigned char _myDataLines;
};

}

#endif 
