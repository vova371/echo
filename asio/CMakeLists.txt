set(CMAKE_CXX_STANDARD 20)
find_package(Boost 1.82.0)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    foreach(proto udp tcp)
    	list(APPEND names ${proto}_coroutines_server)
    endforeach()

    foreach(name ${names})
        message(NOTICE "executable: " ${name})
        add_executable(${name} ${name}.cpp)
	target_link_libraries(${name} PRIVATE Threads::Threads)
    endforeach()
endif()
