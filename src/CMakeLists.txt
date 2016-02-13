add_custom_target(version
    COMMAND ../CreateVersionFile.py .)

add_subdirectory(tess)
add_subdirectory(base)
add_subdirectory(graphics)
add_subdirectory(imaging)
add_subdirectory(oscpack)
add_subdirectory(audio)
add_subdirectory(video)
add_subdirectory(player)

# TODO see Makefile.am and
#   TODO EXTRA_DIST
# TODO anim wrapper python test utils samples
