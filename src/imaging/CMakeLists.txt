if(${AVG_ENABLE_V4L2})
    set(IMAGING_SOURCES V4LCamera.cpp)
    set (IMAGING_LIBS ${JPEG_LIBRARIES})
endif()
if(${AVG_ENABLE_1394_2})
    set(IMAGING_SOURCES ${IMAGING_SOURCES} FWCameraUtils.cpp)
    set (IMAGING_LIBS ${IMAGING_LIBS} ${DC1394_2_LDFLAGS})
endif()

# TODO Untested, windows-specific.
if(${AVG_ENABLE_CMU1394})
    set(IMAGING_SOURCES ${IMAGING_SOURCES} CMUCamera.cpp CMUCameraUtils.cpp)
endif()
if(${AVG_ENABLE_DSHOW})
    set(IMAGING_SOURCES ${IMAGING_SOURCES} DSCamera.cpp DSHelper.cpp DSSampleGrabber.cpp)
endif()

add_library(imaging
    ${IMAGING_SOURCES}
    Camera.cpp FWCamera.cpp FakeCamera.cpp CameraInfo.cpp)
target_include_directories(imaging
    PUBLIC SYSTEM ${Boost_INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS})
target_link_libraries(imaging
    PUBLIC ${IMAGING_LIBS})
