cmake_minimum_required (VERSION 2.6)
project (lacenet)

set (CMAKE_C_FLAGS "-std=gnu99 -Wno-deprecated-declarations ${CMAKE_C_FLAGS}")

set (SOURCES ${SOURCES}
        src/buffer.c
        src/error.c
        src/message.c
        src/client/channel.c
        src/client/client.c
        src/client/message.c
        src/client/peer.c
        src/client/request/channel_list.c
        src/client/request/connect.c
        src/client/request/join_channel.c
        src/client/request/leave_channel.c
        src/client/request/set_name.c
        src/server/channel.c
        src/server/client.c
        src/server/message.c
        src/server/server.c
        src/server/request/channel_list.c
        src/server/request/connect.c
        src/server/request/join_channel.c
        src/server/request/leave_channel.c
        src/server/request/set_name.c
        deps/list.c
        )

include_directories (include/)

set (LIBS ${LIBS} lacewing)

add_library (lacenet ${SOURCES})
add_executable (test_server test/server.c)
add_executable (test_client test/client.c)

set_target_properties(lacenet PROPERTIES VERSION 0.3.0
                                         SOVERSION 0.3)

target_link_libraries (lacenet ${LIBS})
target_link_libraries (test_server lacenet ${LIBS})
target_link_libraries (test_client lacenet ${LIBS})

# MSVC only supports C89, so compile everything as C++ there
#
if (MSVC)
    set_source_files_properties(${SOURCES} test/server.c test/client.c PROPERTIES LANGUAGE CXX)
endif ()

install(TARGETS lacenet DESTINATION lib)
install(FILES include/lacenet.h include/lacenet_client.h include/lacenet_server.h DESTINATION include)