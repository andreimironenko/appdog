# adding the Google_Tests_run target

include(FetchContent)

include_directories(../include)

set(CMAKE_CXX_STANDARD 17)


# linking Google_Tests_run with DateConverter_lib which will be tested
add_executable(simple-daemon simple_daemon.cpp)
target_link_libraries(simple-daemon daemon mq rt)

add_executable(simple-daemon-cli simple_daemon_cli.cpp)
#target_link_libraries(simple-daemon-cli daemon_cli)
