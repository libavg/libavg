set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")

file(GENERATE
     OUTPUT ${SETUP_PY}
     INPUT ${SETUP_PY_IN})

add_custom_target(pybuild ALL DEPENDS avg
                  COMMAND python ${SETUP_PY} build)

install(CODE "execute_process(COMMAND python ${SETUP_PY} install)")
