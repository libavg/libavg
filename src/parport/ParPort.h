//
// $Id$
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
