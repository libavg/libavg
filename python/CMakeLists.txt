set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")

get_property(AVG_SO_LOCATION TARGET avg PROPERTY LOCATION)

configure_file(${SETUP_PY_IN} ${SETUP_PY})

add_custom_target(pybuild ALL DEPENDS avg
                  COMMAND python ${SETUP_PY} build)

install(CODE "execute_process(COMMAND python ${SETUP_PY} install)")
