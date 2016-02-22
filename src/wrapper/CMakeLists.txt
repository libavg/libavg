add_library(avg SHARED
    WrapHelper.cpp avg_wrap.cpp node_wrap.cpp raster_wrap.cpp
    event_wrap.cpp fx_wrap.cpp misc_wrap.cpp vector_wrap.cpp raster2_wrap.cpp
    bitmap_wrap.cpp anim_wrap.cpp)
set_target_properties(avg PROPERTIES PREFIX "")
set_target_properties(avg PROPERTIES SUFFIX ".so") # Make sure we don't generate a .dylib on mac.
target_link_libraries(avg PUBLIC player anim)
