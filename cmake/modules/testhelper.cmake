macro(addTestToLibavgPythonPackage testTarget)
    add_custom_target(copy_${testTarget} ALL)
    add_dependencies(copy_${testTarget} ${testTarget} copy_python_tests)
    add_custom_command(TARGET copy_${testTarget}
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/${testTarget}"
        "${CMAKE_BINARY_DIR}/python/libavg/test/cpptest"
        )

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/baseline")
        add_custom_command(TARGET copy_${testTarget}
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/baseline"
            "${CMAKE_BINARY_DIR}/python/libavg/test/cpptest/baseline"
            )
    endif()
endmacro()

macro(addPluginToLibavgPythonPackage plugin)
    add_custom_target(copy_${plugin} ALL)
    add_dependencies(copy_${plugin} ${plugin} copy_python_tests)
    add_custom_command(TARGET copy_${plugin}
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/${plugin}${CMAKE_SHARED_MODULE_SUFFIX}"
        "${CMAKE_BINARY_DIR}/python/libavg/test/plugin"
        )
endmacro()
