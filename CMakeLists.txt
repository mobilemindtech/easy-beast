set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.10)

project(EasyBeast VERSION 0.0.1 LANGUAGES CXX)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(EasyBeast
    include/asio/round_robin.hpp
    include/asio/yield.hpp
    include/asio/detail/yield.hpp
    include/extra/optional.h
    include/extra/string_view.h
    include/extra/bench.h
    include/extra/final_action.h

    include/http_handler.h
    include/httpserver.h

    public/EasyBeastInterop.h


    src/http_handler.cpp
    src/httpserver.cpp
    src/EasyBeastInterop.cpp
)

set_target_properties(EasyBeast PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER public/EasyBeastInterop.h)

target_link_libraries(EasyBeast
    boost_thread
    boost_fiber
    boost_context
)

#install(TARGETS EasyBeast
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    #RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}o
#)
#
#install(FILES public/interop.h DESTINATION include/${PROJECT_NAME})


target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1)


install(TARGETS ${PROJECT_NAME} EXPORT EasyBeastConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION include)

install(DIRECTORY public/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(EXPORT EasyBeastConfig DESTINATION share/EasyBeast/cmake)

export(TARGETS ${PROJECT_NAME} FILE EasyBeastConfig.cmake)
