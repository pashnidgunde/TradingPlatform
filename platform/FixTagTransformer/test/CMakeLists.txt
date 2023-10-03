project(TestFixTagTransformer)


find_package(GTest CONFIG REQUIRED)

add_executable (${PROJECT_NAME}
	TestFixTagTransformer.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../include)
target_link_libraries(${PROJECT_NAME} PRIVATE FixTagTransformer GTest::gtest GTest::gtest_main)


add_test (NAME ${PROJECT_NAME} COMMAND TestFixTagTransformer)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tests")
