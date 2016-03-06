add_subdirectory(plugin)

add_executable(testbase testbase.cpp)
target_link_libraries(testbase base)
add_test(NAME testbase COMMAND testbase)

add_executable(testgraphics GraphicsTest.cpp testgraphics.cpp)
target_link_libraries(testgraphics graphics)
add_test(NAME testgraphics COMMAND testgraphics ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(testgpu testgpu.cpp GraphicsTest.cpp)
target_link_libraries(testgpu graphics)
add_test(NAME testgpu COMMAND testgpu ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(testplayer testplayer.cpp)
target_link_libraries(testplayer player)
add_test(NAME testplayer COMMAND testplayer ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(testvideo GraphicsTest.cpp testvideo.cpp)
target_link_libraries(testvideo video)
add_test(NAME testvideo COMMAND testvideo ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(testlimiter testlimiter.cpp)
target_link_libraries(testlimiter audio)
add_test(NAME testlimiter COMMAND testlimiter)

