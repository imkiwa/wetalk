cmake_minimum_required (VERSION 2.6)

PROJECT (wetalk)

set (CMAKE_C_FLAGS "-D_GNU_SOURCE -fPIC --pie -std=c99")
set (CMAKE_EXE_LINKER_FLAGS "-fPIC --pie")

set (DEP_DIR "${PROJECT_SOURCE_DIR}/deps")

set (SRC_qserver "${DEP_DIR}/qserver")

include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories ("${SRC_qserver}/include")

add_subdirectory ("${SRC_qserver}")

add_executable (wetalk
    src/client_main.c
    src/client.c
    src/main.c
    src/server_main.c
    src/server.c
    src/user.c
    src/walloc.c
    src/wetalk.c
  )
target_link_libraries (wetalk
    qserver sqlite3 pthread ncursesw
  )

install (
  TARGETS wetalk
  
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
