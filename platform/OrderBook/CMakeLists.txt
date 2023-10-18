project(OrderBook LANGUAGES CXX)

add_library(${PROJECT_NAME}
        include/OutgoingEvents.h
        include/NodePool.h
        OrderBook.cpp
        include/OrderBook.h
        include/LinkedList.h
        include/NewOrderBook.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ./include)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Platform")

add_subdirectory(test)