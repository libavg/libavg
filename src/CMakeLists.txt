add_custom_target(version
  COMMAND ${CMAKE_SOURCE_DIR}/CreateVersionFile.py .)

add_subdirectory(tess)
add_subdirectory(base)
add_subdirectory(graphics)
add_subdirectory(imaging)
add_subdirectory(oscpack)
add_subdirectory(audio)
add_subdirectory(video)
add_subdirectory(player)
add_subdirectory(anim)
add_subdirectory(wrapper)
add_subdirectory(python)

# TODO see Makefile.am and
#   TODO EXTRA_DIST
# TODO python test utils samples
