add_library(avg SHARED
    WrapHelper.cpp avg_wrap.cpp node_wrap.cpp raster_wrap.cpp
    event_wrap.cpp fx_wrap.cpp misc_wrap.cpp vector_wrap.cpp raster2_wrap.cpp
    bitmap_wrap.cpp anim_wrap.cpp)
# Make sure we don't generate a .dylib on mac.
set_target_properties(avg PROPERTIES PREFIX "")
set_target_properties(avg PROPERTIES SUFFIX ".so")

target_link_libraries(avg PUBLIC player anim)

# If these flags aren't passed to the linker, oscpack's global variables
# don't get exported to plugins.
if(${PLATFORM_LINUX})
target_link_libraries(avg PUBLIC
        -Wl,--whole-archive
        oscpack
        -Wl,--no-whole-archive
        )
else()
target_link_libraries(avg PUBLIC
        -Wl,-force_load oscpack
        )
endif()
