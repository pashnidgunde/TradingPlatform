project (OrderCache LANGUAGES CXX)

find_package(Boost REQUIRED)
include_directories(./include)
add_library (OrderCache
  ./include/OrderCache.h
  ./include/OrderCacheImpl.h
  ./OrderCacheImpl.cpp)

target_include_directories(OrderCache SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
link_directories(${PROJECT_NAME} {Boost_LIBRARY_DIRS})
set_target_properties(OrderCache PROPERTIES FOLDER "Platform")

add_subdirectory(test)
