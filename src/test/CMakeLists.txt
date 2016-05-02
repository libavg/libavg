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
    testapp.py
    testcase.py
    avgwindowconfig.xml
    image.avg
    illustratorRect.svg
    )

set(test_directories
    baseline
    fonts
    extrafonts
    media
    testmediadir
    )


set(python_test_package_dir "${CMAKE_BINARY_DIR}/python/libavg/test")
file(MAKE_DIRECTORY ${python_test_package_dir})
file(MAKE_DIRECTORY "${python_test_package_dir}/cpptest")
file(MAKE_DIRECTORY "${python_test_package_dir}/plugin")

foreach(dir ${directories})
    file(MAKE_DIRECTORY "${python_test_package_dir}/${dir}")
endforeach(dir)

add_custom_target(copy_python_tests ALL)

foreach(res ${test_files})
    add_custom_command(TARGET copy_python_tests
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/${res}" "${python_test_package_dir}"
        )
endforeach()

foreach(res ${test_directories})
    add_custom_command(TARGET copy_python_tests
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/${res}"
        "${python_test_package_dir}/${res}"
        )
endforeach()


add_test(NAME python_test
    COMMAND ${PYTHON_EXECUTABLE} avg_test --python
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/python/scripts"
    )

add_subdirectory(plugin)
