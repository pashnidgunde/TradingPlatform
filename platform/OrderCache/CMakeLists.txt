project (OrderCache LANGUAGES CXX)

find_package(Boost REQUIRED)
include_directories(./include)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_library (OrderCache
  ./include/OrderCache.h
  ./include/OrderCacheImpl.h
  ./OrderCacheImpl.cpp)

link_directories(${PROJECT_NAME} {Boost_LIBRARY_DIRS})
set_target_properties(OrderCache PROPERTIES FOLDER "Platform")

add_subdirectory(test)
