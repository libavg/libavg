function(addTestTargetToLibavgPythonPackage testTarget)
    add_custom_target(copy_${testTarget} ALL)
    add_dependencies(copy_${testTarget} ${testTarget})
    add_custom_command(TARGET copy_${testTarget}
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/${testTarget}"
        "${CMAKE_BINARY_DIR}/python/libavg/test/cpptest"
        )
endfunction()


function(addTestDataToLibavgPythonPackage testTarget dataDir)
    add_custom_command(TARGET copy_${testTarget}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/${dataDir}"
        "${CMAKE_BINARY_DIR}/python/libavg/test/cpptest/baseline"
        )
endfunction()
