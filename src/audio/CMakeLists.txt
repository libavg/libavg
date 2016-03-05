add_library(audio
    AudioEngine.cpp AudioBuffer.cpp AudioParams.cpp AudioMsg.cpp
    AudioSource.cpp)
target_link_libraries(audio
    PUBLIC base)

