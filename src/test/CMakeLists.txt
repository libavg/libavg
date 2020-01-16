include(testhelper)

set(test_files
    AVTest.py
    AnimTest.py
    AppTest.py
    DynamicsTest.py
    EventTest.py
    FXTest.py
    GestureTest.py
    ImageTest.py
    LoggerTest.py
    MultiWindowTest.py
    OffscreenTest.py
    PlayerTest.py
    PluginTest.py
    PythonTest.py
    Test.py
    VectorTest.py
    WidgetTest.py
    WordsTest.py
    camcfgs.py
    checkcamera.py
    checkpoly.py
    avgwindowconfig.xml
    image.avg
    illustratorRect.svg
    )

set(test_directories
    baseline
    fonts
    extrafonts
    media/incompleteSkinMedia
    media
    testmediadir
    )

set(python_test_package_dir "${CMAKE_BINARY_DIR}/python/libavg/test")
file(MAKE_DIRECTORY ${python_test_package_dir})
file(MAKE_DIRECTORY "${python_test_package_dir}/cpptest")

add_custom_target(copy_python_tests ALL)

foreach(res ${test_files})
    add_custom_command(TARGET copy_python_tests
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/${res}" "${python_test_package_dir}"
        )
endforeach()

foreach(dir ${test_directories})
    copyToStaging(${dir})
endforeach()


add_test(NAME python_test
    COMMAND ${PYTHON_EXECUTABLE} Test.py
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/python/libavg/test"
    )

add_subdirectory(plugin)
