
find_package(Threads)
add_executable(lab2 main.c)
target_link_libraries(lab2 ${CMAKE_THREAD_LIBS_INIT})