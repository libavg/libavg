set(PLUGIN_NAME colorplugin)
set(PLUGIN_SRC ColorNode.cpp)

add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SRC})

list(APPEND PLUGIN_INCLUDES ../../player)
list(APPEND PLUGIN_INCLUDES ${PYTHON_INCLUDE_DIRS})
list(APPEND PLUGIN_LIBS ${PYTHON_LIBRARIES})
list(APPEND PLUGIN_INCLUDES ${Boost_INCLUDE_DIRS})
list(APPEND PLUGIN_LIBS ${Boost_LIBRARIES})
list(APPEND PLUGIN_INCLUDES ${LIBXML2_INCLUDE_DIRS})
list(APPEND PLUGIN_LIBS ${LIBXML2_LIBRARIES})
#list(APPEND PLUGIN_INCLUDES ${PANGO_INCLUDE_DIRS})
#list(APPEND PLUGIN_LIBS ${PANGO_LIBRARIES})
#list(APPEND PLUGIN_INCLUDES ${GLIB_INCLUDE_DIR})
#list(APPEND PLUGIN_LIBS ${GLIB_LIBRARIES})

include_directories(${PLUGIN_INCLUDES})

#target_compile_definitions(${PLUGIN_NAME} PUBLIC AVG_PLUGIN)
set_target_properties(${PLUGIN_NAME} PROPERTIES PREFIX "")

if (PLATFORM_LINUX)
    target_link_libraries(
        ${PLUGIN_NAME} ${PLUGIN_LIBS} ../../wrapper/.libs/avg.so
        )

elseif (PLATFORM_MACOSX)
    target_link_libraries(${PLUGIN_NAME} -bundle -flat_namespace "-undefined suppress")
    target_link_libraries(${PLUGIN_NAME} ${PLUGIN_LIBS})

elseif (WIN32)
    target_link_libraries(
        ${PLUGIN_NAME} ${PLUGIN_LIBS} ../../../win/Release/avg.lib
        )
endif()

