project(ClientApp LANGUAGES CXX)

find_package(Boost REQUIRED)

add_executable(${PROJECT_NAME}
        CSVFileReader.h
        Tokenizer.h
        Client.cpp
)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
link_directories(${PROJECT_NAME} {Boost_LIBRARY_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "service")
