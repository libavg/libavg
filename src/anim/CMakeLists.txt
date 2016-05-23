add_library(anim
    Anim.cpp SimpleAnim.cpp LinearAnim.cpp AttrAnim.cpp ContinuousAnim.cpp
    EaseInOutAnim.cpp WaitAnim.cpp ParallelAnim.cpp StateAnim.cpp)
target_link_libraries(anim
    PUBLIC base ${PYTHON_LIBRARIES})
target_include_directories(anim
    PUBLIC SYSTEM ${PYTHON_INCLUDE_DIRS})
