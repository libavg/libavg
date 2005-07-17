//
// $Id$
//

#include "../parport/ParPort.h"
#include "../conradrelais/ConradRelais.h"

#include <boost/python.hpp>
#include <linux/parport.h>

using namespace boost::python;
using namespace avg;

enum ControlLines {
    CONTROL_STROBE = PARPORT_CONTROL_STROBE,
    CONTROL_AUTOFD = PARPORT_CONTROL_AUTOFD,
    CONTROL_INIT = PARPORT_CONTROL_INIT,
    CONTROL_SELECT = PARPORT_CONTROL_SELECT
};

enum StatusLines {
    STATUS_ERROR = PARPORT_STATUS_ERROR,
    STATUS_SELECT = PARPORT_STATUS_SELECT,
    STATUS_PAPEROUT = PARPORT_STATUS_PAPEROUT,
    STATUS_ACK = PARPORT_STATUS_ACK,
    STATUS_BUSY = PARPORT_STATUS_BUSY
};

void export_devices()
{
    enum_<ControlLines>("controllines")
        .value("CONTROL_STROBE", CONTROL_STROBE)
        .value("CONTROL_AUTOFD", CONTROL_AUTOFD)
        .value("CONTROL_INIT", CONTROL_INIT)
        .value("CONTROL_SELECT", CONTROL_SELECT)
        .export_values();

    enum_<StatusLines>("statuslines")
        .value("STATUS_ERROR", STATUS_ERROR)
        .value("STATUS_SELECT", STATUS_SELECT)
        .value("STATUS_PAPEROUT", STATUS_PAPEROUT)
        .value("STATUS_ACK", STATUS_ACK)
        .value("STATUS_BUSY", STATUS_BUSY)
        .export_values();

    enum_<ParPortData>("DataBits")
        .value("PARPORTDATA0", BIT0)
        .value("PARPORTDATA1", BIT1)
        .value("PARPORTDATA2", BIT2)
        .value("PARPORTDATA3", BIT3)
        .value("PARPORTDATA4", BIT4)
        .value("PARPORTDATA5", BIT5)
        .value("PARPORTDATA6", BIT6)
        .value("PARPORTDATA7", BIT7)
        .export_values();
    
    class_<ParPort>("ParPort")
        .def("init", &ParPort::init)
        .def("setControlLine", &ParPort::setControlLine)
        .def("getStatusLine", &ParPort::getStatusLine)
        .def("setDataLines", &ParPort::setDataLines)
        .def("clearDataLines", &ParPort::clearDataLines)
        .def("isAvailable", &ParPort::isAvailable)
        ;

    class_<ConradRelais>("ConradRelais", init<Player*, int>())
        .def("getNumCards", &ConradRelais::getNumCards)
        .def("set", &ConradRelais::set)
        .def("get", &ConradRelais::get);
}
