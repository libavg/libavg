# A refresh of this search (eg: new virtualenv) can be obtained by running cmake
# with -UPYTHON_INTERPRETER
find_program(PYTHON_INTERPRETER python)

message(STATUS "Using python intepreter: ${PYTHON_INTERPRETER}")

set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
set(SETUP_PY_CF "${CMAKE_CURRENT_BINARY_DIR}/setup.py.cf")
set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")

configure_file("${SETUP_PY_IN}" "${SETUP_PY_CF}")

file(GENERATE
     OUTPUT "${SETUP_PY}"
     INPUT "${SETUP_PY_CF}")

add_custom_target(python-package ALL DEPENDS avg
                  COMMAND ${PYTHON_INTERPRETER} ${SETUP_PY} build)

install(CODE "execute_process(COMMAND ${PYTHON_INTERPRETER} ${SETUP_PY} install
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