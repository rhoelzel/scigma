enable_language(Fortran)
include_directories(../external/tinythread)
add_library(num SHARED function.cpp operator.cpp parser.cpp
		equationsystem.cpp internalequationsystem.cpp externalequationsystem.cpp
		odessa.cpp
		stepper.cpp integrationstepper.cpp mapstepper.cpp) 
set_target_properties(num PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(num odessa)
add_subdirectory(test)
