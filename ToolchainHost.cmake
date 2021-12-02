set(CMAKE_CXX_FLAGS "-O0 -g -Wall")
set(USE_QT 1)

if (WIN32)
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll" ".dll.a" ".lib" ".a")
endif (WIN32)

function(add_mcu_executable NAME MCU)
    add_executable(${NAME} src/_empty.cpp)
endfunction(add_mcu_executable)

function(add_host_executable NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} PRIVATE Qt5::Widgets ${PNG_LIBRARY})
endfunction(add_host_executable)

function(add_cubemx_project NAME MCU)
    add_executable(${NAME} src/_empty.cpp)
endfunction(add_cubemx_project)


