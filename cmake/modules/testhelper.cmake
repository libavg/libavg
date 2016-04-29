macro(addTestToLibavgPythonPackage testTarget)
    add_custom_target(copy_${testTarget} ALL)
    add_dependencies(copy_${testTarget} ${testTarget} copy_python_tests)
    add_custom_command(TARGET copy_${testTarget}
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/${testTarget}"
        "${CMAKE_BINARY_DIR}/python/libavg/test/cpptest"
        )
endmacro(addTestToLibavgPythonPackage)
