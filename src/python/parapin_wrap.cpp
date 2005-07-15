//
// $Id$
//

#include <parapin.h>

#include <boost/python.hpp>

using namespace boost::python;

enum PIN 
{
    PIN01 = LP_PIN01,
    PIN02 = LP_PIN02,
    PIN03 = LP_PIN03,
    PIN04 = LP_PIN04,
    PIN05 = LP_PIN05,
    PIN06 = LP_PIN06,
    PIN07 = LP_PIN07,
    PIN08 = LP_PIN08,
    PIN09 = LP_PIN09,
    PIN10 = LP_PIN10,
    PIN11 = LP_PIN11,
    PIN12 = LP_PIN12,
    PIN13 = LP_PIN13,
    PIN14 = LP_PIN14,
    PIN15 = LP_PIN15,
    PIN16 = LP_PIN16,
    PIN17 = LP_PIN17
};

BOOST_PYTHON_MODULE(parapin)
{
    def("set_pin", set_pin);
    def("clear_pin", clear_pin);
    enum_<PIN>("PIN")
        .value("PIN01", PIN01)
        .value("PIN02", PIN02)
        .value("PIN03", PIN03)
        .value("PIN04", PIN04)
        .value("PIN05", PIN05)
        .value("PIN06", PIN06)
        .value("PIN07", PIN07)
        .value("PIN08", PIN08)
        .value("PIN09", PIN09)
        .value("PIN10", PIN10)
        .value("PIN11", PIN11)
        .value("PIN12", PIN12)
        .value("PIN13", PIN13)
        .value("PIN14", PIN14)
        .value("PIN15", PIN15)
        .value("PIN16", PIN16)
        .value("PIN17", PIN17);
}
