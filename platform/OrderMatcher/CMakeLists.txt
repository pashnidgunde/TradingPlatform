project(OrderMatcher LANGUAGES CXX)
find_package(Boost REQUIRED)

add_library(${PROJECT_NAME}
        include/SymbolResolver.h
        include/MessageQueue.h
        OrderMatcher.cpp
        include/Journal.h
        include/TSQueue.h
        include/NewMessageHandler.h
)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ../OrderBook/include)
target_include_directories(${PROJECT_NAME} PRIVATE ./include)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Platform")