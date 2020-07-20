add_library(base
    FileHelper.cpp Exception.cpp Logger.cpp
    ConfigMgr.cpp XMLHelper.cpp TimeSource.cpp OSHelper.cpp
    ProfilingZone.cpp ThreadProfiler.cpp ScopeTimer.cpp Test.cpp
    TestSuite.cpp ObjectCounter.cpp Directory.cpp DirEntry.cpp
    StringHelper.cpp MathHelper.cpp GeomHelper.cpp CubicSpline.cpp
    BezierCurve.cpp UTF8String.cpp Triangle.cpp Polygon.cpp DAG.cpp WideLine.cpp
    Backtrace.cpp ProfilingZoneID.cpp GLMHelper.cpp
    StandardLogSink.cpp ThreadHelper.cpp
)
target_compile_options(base
    PUBLIC ${LIBXML2_CFLAGS})
target_include_directories(base
    PUBLIC SYSTEM ${Boost_INCLUDE_DIRS})
target_link_libraries(base
    PUBLIC tess ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} ${LIBXML2_LDFLAGS})
# a dependency to boost-python is added on some systems (e.g. Debian, Arch Linux,
# macOS), which breaks linking (Linux) or runtime loading (macOS) w/o python libs here
target_link_libraries(base
    PUBLIC ${PYTHON_LIBRARIES})

if (RT_LIBRARY)
    target_include_directories(base
        PUBLIC SYSTEM ${RT_INCLUDE_DIR})
    target_link_libraries(base
        PUBLIC ${RT_LIBRARY})
endif()

link_libraries(base)
add_executable(testbase testbase.cpp)
set_target_properties(testbase PROPERTIES ENABLE_EXPORTS TRUE) # cmake >= 3.4 (CMP0065) - required for Backtrace
add_test(NAME testbase
    COMMAND ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest/testbase
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python/libavg/test/cpptest)

include(testhelper)
copyTestToStaging(testbase)
