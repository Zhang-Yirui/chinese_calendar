cmake_minimum_required(VERSION 3.28)
project(calendar C)

set(CMAKE_C_STANDARD 23)

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fexec-charset=GBK")
    set(MACHINE_SYSTEM "windows")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(MACHINE_SYSTEM "linux")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(MACHINE_SYSTEM "macos")
ELSE ()
    set(MACHINE_SYSTEM "unknown")
ENDIF ()

set(TARGET_NAME "calendar_${MACHINE_SYSTEM}")

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB INCLUDES ${PROJECT_SOURCE_DIR}/inc/*.h)
include_directories("${PROJECT_SOURCE_DIR}/inc")

add_executable(${TARGET_NAME} main.c ${SOURCES} ${INCLUDES})