add_library(audio
    AudioEngine.cpp AudioBuffer.cpp AudioParams.cpp AudioMsg.cpp
    AudioSource.cpp)
target_link_libraries(audio
    PUBLIC base)

link_libraries(audio)
add_executable(testlimiter testlimiter.cpp)
add_test(NAME testlimiter COMMAND testlimiter)
