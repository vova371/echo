set(CMAKE_CXX_STANDARD 20)
find_package(Boost 1.82.0)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    foreach(proto udp tcp)
    	list(APPEND names ${proto}_coroutines_server)
    endforeach()

    foreach(name ${names})
        message(NOTICE "executable: " ${name})
        add_executable(${name} ${name}.cpp)
    endforeach()
endif()
