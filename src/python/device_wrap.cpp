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

#include "../avgconfig.h"
#ifdef AVG_ENABLE_PARPORT
#include "../parport/ParPort.h"
#endif
#include "../conradrelais/ConradRelais.h"

#include <boost/python.hpp>
#ifdef AVG_ENABLE_PARPORT
#include <linux/parport.h>
#endif

using namespace boost::python;
using namespace avg;

#ifdef AVG_ENABLE_PARPORT
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
#endif

void export_devices()
{
#ifdef AVG_ENABLE_PARPORT
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
    
    class_<ParPort>("ParPort",
            "Used for low-level control of the parallel port's data, status and control\n"
            "lines.")
        .def("init", &ParPort::init,
                "init(DeviceName) -> None\n\n"
                "Opens the given device as a parallel port. If DeviceName is an empty\n"
                "string, /dev/parport0 is used as device name.")
        .def("setControlLine", &ParPort::setControlLine,
                "setControlLine(line, value) -> ok\n\n",
                "Sets or clears one of the control lines. Possible values for line are\n"
                "CONTROL_STROBE, CONTROL_AUTOFD, CONTROL_INIT and CONTROL_SELECT.\n"
                "Returns 1 if the value was set successfully, 0 otherwise.")
        .def("getStatusLine", &ParPort::getStatusLine,
                "getStatusLine(line) -> value\n\n"
                "Returns the value of one of the parallel port status lines. Possible\n"
                "lines are STATUS_ERROR, STATUS_SELECT, STATUS_PAPEROUT, STATUS_ACK\n"
                "and STATUS_BUSY.")
        .def("setDataLines", &ParPort::setDataLines,
                "setDataLines(lines) -> ok\n\n"
                "Sets the data lines given as argument. Constants to used for these\n"
                "lines are PARPORTDATA0-PARPORTDATA7. Several of these constants can\n"
                "be or'ed together to set several lines. The lines not mentioned in\n"
                "the parameter are left unchanged. Returns 1 if the lines were set,\n"
                "0 otherwise.")
        .def("clearDataLines", &ParPort::clearDataLines,
                "clearDataLines(lines) -> ok\n\n"
                "Clears the data lines given as argument. Constants to used for these\n"
                "lines are PARPORTDATA0-PARPORTDATA7. Several of these constants can\n"
                "be or'ed together to clear several lines. The lines not mentioned in\n"
                "the parameter are left unchanged.")
        .def("setAllDataLines", &ParPort::setDataLines,
                "setAllDataLines(lines) -> ok\n\n"
                "Sets and resets all data lines according to the bits set in the\n"
                "argument. Constants to used for these\n"
                "lines are PARPORTDATA0-PARPORTDATA7. Several of these constants can\n"
                "be or'ed together to set several lines. The lines not mentioned in\n"
                "the parameter are set to 0. Returns 1 if the lines were set,\n"
                "0 otherwise.")
        .def("isAvailable", &ParPort::isAvailable,
                "isAvailable() -> ok\n\n"
                "Returns 1 if the parallel port has been opened successfully, 0\n"
                "otherwise.")
        ;
#endif

    class_<ConradRelais>("ConradRelais",
            "Interface to one or more conrad relais cards connected to a serial port.\n"
            "Per card, up to eight 220V devices can be connected.",
            init<Player*, int>(
                "ConradRelais(AVGPlayer, port)\n\n"
                "Opens a connection to the relais card(s) connected to the port given.\n"
                "port is an integer. The actual device opened is /dev/ttyS<port>."))
        .def("getNumCards", &ConradRelais::getNumCards,
                "getNumCards() -> num\n\n"
                "Returns the number of cards connected to the serial port.")
        .def("set", &ConradRelais::set,
                "set(card, index, value) -> None\n\n"
                "Sets or resets one of the relais. card and index select the relais\n"
                "to set.")
        .def("get", &ConradRelais::get,
                "get(card, index) -> value\n\n"
                "Returns the state of one of the relais. card and index select the\n"
                "relais to query.");
}
