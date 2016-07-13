add_library(avg SHARED
    WrapHelper.cpp avg_wrap.cpp node_wrap.cpp raster_wrap.cpp
    event_wrap.cpp fx_wrap.cpp misc_wrap.cpp vector_wrap.cpp raster2_wrap.cpp
    bitmap_wrap.cpp anim_wrap.cpp)
# Make sure we don't generate a .dylib on mac.
set_target_properties(avg PROPERTIES PREFIX "")
set_target_properties(avg PROPERTIES SUFFIX ".so")

# If these flags aren't passed to the linker under linux, oscpack's global variables
# don't get exported to plugins.
if(${PLATFORM_LINUX})
    set(LINK_EXPORT_ALL_FLAG "-Wl,--whole-archive")
    set(LINK_UNEXPORT_ALL_FLAG "-Wl,--no-whole-archive")
else()
    set(LINK_EXPORT_ALL_FLAG "")
    set(LINK_UNEXPORT_ALL_FLAG "")
endif()


target_link_libraries(avg PUBLIC
        ${LINK_EXPORT_ALL_FLAG}
        player anim oscpack
        ${LINK_UNEXPORT_ALL_FLAG}
        )
