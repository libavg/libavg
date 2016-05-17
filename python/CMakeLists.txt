# A refresh of this search (eg: new virtualenv) can be obtained by running cmake
# with -UPYTHON_INTERPRETER
find_program(PYTHON_INTERPRETER python)

message(STATUS "Using python intepreter: ${PYTHON_INTERPRETER}")

# Out of source build requires a copy of package, setup.py and scripts
if(NOT CMAKE_CURRENT_SOURCE_DIR EQUAL CMAKE_CURRENT_BINARY_DIR)
    message(STATUS "Copying python package to build directory")

    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/libavg"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts"
         DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in" "${CMAKE_CURRENT_BINARY_DIR}/setup.py")

file(COPY "${CMAKE_SOURCE_DIR}/src/graphics/shaders"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/libavg")

add_custom_target(python-build ALL DEPENDS avg
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:avg>"
            "${CMAKE_CURRENT_BINARY_DIR}/libavg/"
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