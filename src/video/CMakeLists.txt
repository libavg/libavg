add_library(video
    FFMpegDemuxer.cpp VideoDemuxerThread.cpp VideoDecoder.cpp
    VideoDecoderThread.cpp AudioDecoderThread.cpp VideoMsg.cpp
    AsyncVideoDecoder.cpp VideoInfo.cpp SyncVideoDecoder.cpp
    FFMpegFrameDecoder.cpp WrapFFMpeg.cpp)
target_link_libraries(video
    PUBLIC base audio graphics ${FFMPEG_LDFLAGS} ${FFMPEG_SWRESAMPLE_LDFLAGS})
target_compile_options(video
    PUBLIC ${FFMPEG_CFLAGS} ${FFMPEG_SWRESAMPLE_CFLAGS})



if(${AVG_ENABLE_VDPAU})
    target_sources(video
        PRIVATE VDPAUDecoder.cpp VDPAUHelper.cpp)
endif()



link_libraries(video)
add_executable(testvideo testvideo.cpp)
add_test(NAME testvideo
    COMMAND ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest/testvideo
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest)

include(testhelper)
copyTestToStaging(testvideo)
copyTestDataToStaging(testvideo baseline)
