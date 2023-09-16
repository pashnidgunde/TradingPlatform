project (FixTagTransformer LANGUAGES CXX)

find_package(Boost REQUIRED)

add_library (FixTagTransformer
  ./include/FixTagTransformer.h
  ./FixTagTransformer.cpp)

target_include_directories(FixTagTransformer PRIVATE ./include)
target_include_directories(FixTagTransformer PUBLIC SYSTEM ${Boost_INCLUDE_DIRS})
	
link_directories(${PROJECT_NAME} {Boost_LIBRARY_DIRS})

set_target_properties(FixTagTransformer PROPERTIES FOLDER "Platform")

add_subdirectory(test)
