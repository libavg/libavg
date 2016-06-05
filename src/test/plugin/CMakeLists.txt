set(PLUGIN_NAME colorplugin)
set(PLUGIN_SRC ColorNode.cpp)

add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SRC})

list(APPEND PLUGIN_INCLUDES ${CMAKE_SOURCE_DIR}/src)

list(APPEND PLUGIN_INCLUDES ${PYTHON_INCLUDE_DIRS})
list(APPEND PLUGIN_LIBS ${PYTHON_LIBRARIES})

list(APPEND PLUGIN_INCLUDES ${Boost_INCLUDE_DIRS})
list(APPEND PLUGIN_LIBS ${Boost_LIBRARIES})

target_compile_options(colorplugin PUBLIC ${LIBXML2_CFLAGS})
target_include_directories(colorplugin PUBLIC ${PLUGIN_INCLUDES})

#target_compile_definitions(${PLUGIN_NAME} PUBLIC AVG_PLUGIN)

set_target_properties(${PLUGIN_NAME} PROPERTIES PREFIX "")
target_compile_options(${PLUGIN_NAME} PUBLIC -Wall -pedantic -std=c++11)

if (${PLATFORM_LINUX})
    target_link_libraries(${PLUGIN_NAME} ${PLUGIN_LIBS} avg)
elseif (${PLATFORM_MACOSX})
    target_link_libraries(${PLUGIN_NAME} -bundle -flat_namespace "-undefined suppress")
    target_link_libraries(${PLUGIN_NAME} ${LIBXML2_LDFLAGS} ${PLUGIN_LIBS})
endif()


add_custom_command(TARGET colorplugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/colorplugin${CMAKE_SHARED_MODULE_SUFFIX}"
            "${CMAKE_BINARY_DIR}/python/libavg/test/plugin"
    )

