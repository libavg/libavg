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

function(copyToStaging dir)
    set(python_test_package_dir "${CMAKE_BINARY_DIR}/python/libavg/test")
    add_custom_command(TARGET copy_python_tests
        COMMAND ${CMAKE_COMMAND}
                ARGS -E make_directory "${python_test_package_dir}/${dir}"
        )
    file(MAKE_DIRECTORY "${python_test_package_dir}/${dir}")
    file(GLOB test_files
            RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/${dir}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*")
    foreach(cur_file ${test_files})
        add_custom_command(TARGET copy_python_tests
            COMMAND ${CMAKE_COMMAND}
            ARGS -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${cur_file}"
                    "${python_test_package_dir}/${dir}/${cur_file}"
            )
    endforeach()
endfunction()
