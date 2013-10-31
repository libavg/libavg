ALL_H = IpEndpointName.h NetworkingUtils.h PacketListener.h TimerListener.h UdpSocket.h \
        MessageMappingOscPacketListener.h OscPacketListener.h OscException.h \
        OscPrintReceivedElements.h OscHostEndianness.h OscReceivedElements.h \
        OscOutboundPacketStream.h OscTypes.h
ALL_CPP = IpEndpointName.cpp  NetworkingUtils.cpp UdpSocket.cpp \
        OscOutboundPacketStream.cpp OscReceivedElements.cpp \
        OscPrintReceivedElements.cpp OscTypes.cpp

EXTRA_DIST = CHANGES LICENSE README TODO

noinst_LTLIBRARIES = liboscpack.la
liboscpack_la_SOURCES = $(ALL_CPP) $(ALL_H)
