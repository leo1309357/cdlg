cmake_minimum_required(VERSION 3.15)

project(cdlg
    VERSION 1.0.0
    DESCRIPTION "CDLG dialogue script parser and runtime"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cdlg STATIC
    src/Database.cpp
    src/Parser.cpp
    src/Session.cpp
)

target_include_directories(cdlg
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(cdlg-cli
    tools/cdlg_cli.cpp
)

target_link_libraries(cdlg-cli PRIVATE cdlg)

install(TARGETS cdlg cdlg-cli
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
