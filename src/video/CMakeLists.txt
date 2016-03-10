add_library(video
    FFMpegDemuxer.cpp VideoDemuxerThread.cpp VideoDecoder.cpp
    VideoDecoderThread.cpp AudioDecoderThread.cpp VideoMsg.cpp
    AsyncVideoDecoder.cpp VideoInfo.cpp SyncVideoDecoder.cpp
    FFMpegFrameDecoder.cpp WrapFFMpeg.cpp)
target_link_libraries(video
    PUBLIC base audio graphics ${FFMPEG_LDFLAGS})
target_compile_options(video
    PUBLIC ${FFMPEG_CFLAGS})



if(${AVG_ENABLE_VDPAU})
    target_sources(video
        PRIVATE VDPAUDecoder.cpp VDPAUHelper.cpp)
endif()



link_libraries(video)
add_executable(testvideo testvideo.cpp)
add_test(NAME testvideo COMMAND testvideo)

# TODO EXTRA_DIST
