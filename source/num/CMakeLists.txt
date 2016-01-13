add_library(num SHARED function.cpp operator.cpp parser.cpp equationsystem.cpp internalequationsystem.cpp externalequationsystem.cpp) 
set_target_properties(num PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
add_subdirectory(test)
