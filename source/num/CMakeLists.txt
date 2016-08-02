enable_language(Fortran)
include_directories(../external/tinythread)
include_directories(../external/loki)
add_library(num STATIC function.cpp operator.cpp parser.cpp
		equationsystem.cpp internalequationsystem.cpp externalequationsystem.cpp
		odessa.cpp
		stepper.cpp integrationstepper.cpp mapstepper.cpp auto.cpp
		poincarestepper.cpp mapmanifoldstepper.cpp
		newton.cpp eigen.cpp function.cpp operator.cpp task.cpp
		guessingwrapper.cpp
		iterationwrapper.cpp
		continuationwrapper.cpp
		equationsystemwrapper.cpp
		manifoldwrapper.cpp
		taskwrapper.cpp) 
set_target_properties(num PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(num tinythread common odessa auto)
add_subdirectory(test)
