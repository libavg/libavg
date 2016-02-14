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
target_include_directories(base
    PUBLIC ${Boost_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})
target_link_libraries(base
    PUBLIC tess ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} ${LIBXML2_LIBRARIES})

link_libraries(base)
add_executable(testbase testbase.cpp)
add_test(NAME testbase COMMAND testbase)
