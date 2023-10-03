project (OrderBook LANGUAGES CXX)

add_library (${PROJECT_NAME}
    include/LinkedList.h
    include/MatchingFields.h
    include/OrderBook.h
    OrderBook.cpp
    include/Order.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ./include)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Platform")

add_subdirectory(test)