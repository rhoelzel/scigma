set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")	

include_directories(../../external/catch)
add_executable(utest utest.cpp
		     internalequationsystem_test.cpp
		     externalequationsystem_test.cpp
		     externalequationsystem_test_p.cpp
		     externalequationsystem_test_t.cpp
		     externalequationsystem_test_pt.cpp				
		     externalequationsystem_test_shared.cpp
		     odessa_test.cpp
		     integrationstepper_test.cpp
		     mapstepper_test.cpp) 
target_link_libraries(utest num tinythread)
