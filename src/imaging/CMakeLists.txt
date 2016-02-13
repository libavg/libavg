add_library(imaging
    Camera.cpp FWCamera.cpp FakeCamera.cpp CameraInfo.cpp)



if(${AVG_ENABLE_V4L2})
    target_sources(imaging
        PRIVATE V4LCamera.cpp)
endif()
if(${AVG_ENABLE_1394_2})
    target_sources(imaging
        PRIVATE FWCameraUtils.cpp)
    target_link_libraries(imaging
        PUBLIC ${DC1394_2_LDFLAGS})
endif()
# TODO these CMU1394 and DSHOW are not mentioned in the Makefile.am
#      but in EXTRA_DIST. I guess I should add the sources appropriatly.
#      This needs testing on intended platforms. Windows, I guess.
if(${AVG_ENABLE_CMU1394})
    target_sources(imaging
        PRIVATE CMUCamera.cpp CMUCameraUtils.cpp)
endif()
if(${AVG_ENABLE_DSHOW})
    target_sources(imaging
        PRIVATE DSCamera.cpp DSHelper.cpp DSSampleGrabber.cpp)
endif()
