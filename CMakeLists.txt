cmake_minimum_required (VERSION 3.0)
project (echo_servers)

add_subdirectory(asio)

foreach(proto udp tcp)
  foreach(type blocking spinning)
    foreach(side client server)
      list(APPEND names ${proto}_${type}_${side})
    endforeach()
  endforeach()
endforeach()

foreach(name ${names})
    message(NOTICE "executable: " ${name})
    add_executable(${name} ${name}.c)
endforeach()
