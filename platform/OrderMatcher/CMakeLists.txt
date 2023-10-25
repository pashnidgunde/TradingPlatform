project(OrderMatcher LANGUAGES CXX)
find_package(Boost REQUIRED)

add_library(${PROJECT_NAME}
        include/SymbolResolver.h
        OrderMatcher.cpp
        ../OrderBook/include/OrderEventListener.h
        ../../include/TSQueue.h
        include/NewMessageHandler.h
        include/MemoryMappedIO.h
)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ../OrderBook/include)
target_include_directories(${PROJECT_NAME} PRIVATE ./include)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Platform")