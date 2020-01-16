project(tess C)

add_library(tess bucketalloc.c dict.c geom.c mesh.c priorityq.c sweep.c tess.c)

# TODO payload: @uzadow, I didn't spot a difference in output with this option.
#               Can you show me?
#target_compile_options(tess PRIVATE -Wno-format-y2k)
