include(testhelper)

# A refresh of this search (eg: new virtualenv) can be obtained by running cmake
# with -UPYTHON_INTERPRETER
find_program(PYTHON_INTERPRETER python)

message(STATUS "Using python intepreter: ${PYTHON_INTERPRETER}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in" "${CMAKE_CURRENT_BINARY_DIR}/setup.py")

add_custom_target(python-build ALL DEPENDS avg
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:avg>"
            "${CMAKE_CURRENT_BINARY_DIR}/libavg/")

# TODO: Copying graphics/shaders doesn't really belong in this file.
copyDirIfDifferent(python-build
        "${CMAKE_SOURCE_DIR}/src/graphics/shaders"
        "${CMAKE_CURRENT_BINARY_DIR}/libavg/shaders")
copyDirIfDifferent(python-build
        "${CMAKE_CURRENT_SOURCE_DIR}/libavg"
        "${CMAKE_CURRENT_BINARY_DIR}/libavg")
copyDirIfDifferent(python-build
        "${CMAKE_CURRENT_SOURCE_DIR}/libavg/data"
        "${CMAKE_CURRENT_BINARY_DIR}/libavg/data")
copyDirIfDifferent(python-build
        "${CMAKE_CURRENT_SOURCE_DIR}/libavg/app"
        "${CMAKE_CURRENT_BINARY_DIR}/libavg/app")
copyDirIfDifferent(python-build
        "${CMAKE_CURRENT_SOURCE_DIR}/libavg/widget"
        "${CMAKE_CURRENT_BINARY_DIR}/libavg/widget")
copyDirIfDifferent(python-build
        "${CMAKE_CURRENT_SOURCE_DIR}/scripts"
        "${CMAKE_CURRENT_BINARY_DIR}/scripts")
add_custom_command(TARGET python-build
        COMMAND ${PYTHON_INTERPRETER} setup.py build)

install(CODE "execute_process(COMMAND ${PYTHON_INTERPRETER} setup.py install
              WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")

# How do I get Clion to run an install target?
#  http://stackoverflow.com/questions/33788729/how-do-i-get-clion-to-run-an-install-target
#
# How do I get Clion to install into virtualenv?
#  Either run clion.sh from an activated virtualenv shell
#  or change PYTHON_INTERPRETER in CMake Cache to the virtualenv python path.
#
add_custom_target(install-for-clion
        make install
        DEPENDS python-package
        COMMENT "Installing libavg via setup.py.")
