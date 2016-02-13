add_library(anim
    Anim.cpp SimpleAnim.cpp LinearAnim.cpp AttrAnim.cpp ContinuousAnim.cpp
    EaseInOutAnim.cpp WaitAnim.cpp ParallelAnim.cpp StateAnim.cpp)
target_link_libraries(anim
    PUBLIC base ${PYTHON_LDFLAGS})
target_compile_options(anim
    PUBLIC ${PYTHON_CFLAGS})
