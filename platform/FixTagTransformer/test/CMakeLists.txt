project(TestFixTagTransformer)
enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable (TestFixTagTransformer
	TestFixTagTransformer.cpp)

target_include_directories(TestFixTagTransformer PRIVATE ../include)
target_link_libraries(TestFixTagTransformer PRIVATE FixTagTransformer GTest::gtest GTest::gtest_main)

add_test(AllTestsInMain TestFixTagTransformer)

set_target_properties(TestFixTagTransformer PROPERTIES FOLDER "Tests")
