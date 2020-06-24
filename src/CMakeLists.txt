add_custom_target(version
  COMMAND ${CMAKE_SOURCE_DIR}/CreateVersionFile.py .)

install(DIRECTORY ./
        DESTINATION include/libavg
        FILES_MATCHING PATTERN *.inl)
install(DIRECTORY ./
        DESTINATION include/libavg
        FILES_MATCHING PATTERN *.hpp)
install(DIRECTORY ./
        DESTINATION include/libavg
        FILES_MATCHING PATTERN *.h)
install(FILES ${CMAKE_BINARY_DIR}/src/avgconfig.h ${CMAKE_BINARY_DIR}/src/version.h
        DESTINATION include/libavg)

add_subdirectory(tess)
add_subdirectory(test)
add_subdirectory(base)
add_subdirectory(graphics)
add_subdirectory(imaging)
add_subdirectory(oscpack)
add_subdirectory(audio)
add_subdirectory(video)
add_subdirectory(player)
add_subdirectory(anim)
add_subdirectory(wrapper)
add_subdirectory(samples)
