project(TestFixTagTransformer)


find_package(GTest CONFIG REQUIRED)

add_executable (TestFixTagTransformer
	TestFixTagTransformer.cpp)

target_include_directories(TestFixTagTransformer PRIVATE ../include)
target_link_libraries(TestFixTagTransformer PRIVATE FixTagTransformer GTest::gtest GTest::gtest_main)


add_test (NAME TestFixTagTransformer COMMAND TestFixTagTransformer)

set_target_properties(TestFixTagTransformer PROPERTIES FOLDER "Tests")
