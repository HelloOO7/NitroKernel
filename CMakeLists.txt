cmake_minimum_required(VERSION 3.0.0)

message("NitroKernel target platform: " ${K_PLATFORM})

IF (K_PLATFORM STREQUAL "ARMv5T")
message("Building for ARMv5T")

set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)
set(CMAKE_C_FLAGS "--specs=nosys.specs")
set(CMAKE_CXX_FLAGS "--specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "-r -march=armv5t -mthumb -mthumb-interwork" CACHE INTERNAL "")

add_compile_options(-r -march=armv5t -mthumb -mthumb-interwork -fno-rtti -fno-exceptions -fvisibility=hidden)

add_compile_definitions(GFL)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_compile_definitions(DEBUG)
endif()

ELSE ()
message( FATAL_ERROR "Invalid target platform!")

ENDIF ()

set(EXL_DESMUME ${K_DESMUME})
set(RPM_PLATFORM ${K_PLATFORM})
set(EXL_PLATFORM ${K_PLATFORM})

project(NitroKernel VERSION 0.1.0)

include(CTest)
enable_testing()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../extlib ${CMAKE_CURRENT_BINARY_DIR}/extlib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libRPM ${CMAKE_CURRENT_BINARY_DIR}/libRPM)

file(GLOB K_SOURCES
    src/*.cpp
    include/*.h
)

add_executable(NitroKernel.elf ${K_SOURCES})
target_link_libraries(NitroKernel.elf PRIVATE ExtLib.Include)
target_link_libraries(NitroKernel.elf PUBLIC LibRPM.Include ExtLib.Heap ExtLib.ABI ExtLib.Base)
include_directories(../swan include)

add_library(NitroKernel.Include include/k_DllExport.h)
set_target_properties(NitroKernel.Include PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(NitroKernel.Include PUBLIC LibRPM.Include)
target_include_directories(NitroKernel.Include PUBLIC include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
