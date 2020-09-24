add_library(oscpack
    IpEndpointName.cpp  NetworkingUtils.cpp UdpSocket.cpp
    OscOutboundPacketStream.cpp OscReceivedElements.cpp
    OscPrintReceivedElements.cpp OscTypes.cpp)

# TODO payload: understand cmake distribution and autoconf EXTRA_DIST and translate!
# EXTRA_DIST = CHANGES LICENSE README TODO
