cmake_minimum_required(VERSION 3.14)

include(FetchContent)
#include(tags.cmake)

project(appdog)

# Add the cmake folder to the FindSphinx module 
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS YES)

include_directories(include)
include_directories(src)

#add_library(mq STATIC mq/mq.cpp include/mq.h mq/mq_.h mq/mq_.cpp)
#add_library(daemon STATIC src/daemon_base.cpp include/daemon.h src/daemon_base_.cpp src/daemon_base_.h)
#add_library(daemon-cli STATIC src/daemon_cli.cpp include/daemon_cli.h src/daemon_cli_.cpp src/daemon_cli_.h)
#
#target_link_libraries(daemon mq boost_program_options)
#target_link_libraries(daemon-cli mq nlohmann_json)
#

#add_executable(simple-daemon-cli simple_daemon_cli.cpp)
#target_link_libraries(simple-daemon-cli daemon_cli)


FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_TAG master)

FetchContent_GetProperties(json)

if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR})
    include_directories(${json_SOURCE_DIR}/include)
endif()

find_package(PythonLibs REQUIRED)
message("Include dirs of Python: " ${PYTHON_INCLUDE_DIRS})
get_filename_component(PYTHON_LIBRARIES_PATH ${PYTHON_LIBRARIES} DIRECTORY)
get_filename_component(PYTHON_LIBRARIES_NAME ${PYTHON_LIBRARIES} NAME)
message("Path Libs of Python: " ${PYTHON_LIBRARIES_PATH})
message("Python Libs: " ${PYTHON_LIBRARIES_NAME})

include_directories(${PYTHON_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARIES})

add_subdirectory(tests)
#add_subdirectory(examples)
add_subdirectory(timer)
add_subdirectory(docs)

# linking Google_Tests_run with DateConverter_lib which will be tested
add_executable(appdogd src/appdogd.cpp src/appdogd.h include/appdog.h)
target_link_libraries(appdogd timer rt dl pthread boost_program_options nlohmann_json::nlohmann_json)

add_library(appdog SHARED
        src/client.cpp
        include/client.h
        src/client_.cpp
        src/client_.h
        src/messages/message.h
        src/messages/activate.h
        src/messages/deactivate.h
        src/messages/kick.h
        )

target_link_libraries(appdog rt pthread
  boost_program_options
  boost_python38
  python3.8
  nlohmann_json::nlohmann_json)
