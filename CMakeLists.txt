cmake_minimum_required (VERSION 3.21)

project ("soft80")

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCE
	"source/main.cpp"
	"source/soft80.cpp"
	"include/soft80.h"
	"include/decoder.h"
	"include/registers.h"
	"source/decoder.cpp"
	"include/memorymap.h"
	"source/registers.cpp"
	"include/devicemap.h"
	"include/util.h"
	"source/util.cpp"
	"include/terminaldevice.h"
 )

add_executable (${PROJECT_NAME} ${SOURCE})

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	"include"
)