project(TestOrderCache)

find_package(GTest CONFIG REQUIRED)

add_executable (TestOrderCache
	TestOrderCache.cpp)

target_include_directories(TestOrderCache PRIVATE ../include)
target_link_libraries(TestOrderCache PRIVATE OrderCache GTest::gtest GTest::gtest_main)
  

add_test (NAME TestOrderCache COMMAND TestOrderCache)

set_target_properties(TestOrderCache PROPERTIES FOLDER "Tests")