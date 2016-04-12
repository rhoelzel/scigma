include_directories(../external/tinythread)
add_library(common SHARED log.cpp blob.cpp util.cpp)
set_target_properties(common PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
add_subdirectory(test)
