cmake_minimum_required (VERSION 2.8)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/common.cmake)

use_cxx11()
set_default_build_type("Release")


set(TARGET "comp_normal")

project(${TARGET})




add_executable (${TARGET} main.cpp)


setup_intel_opencl(${TARGET})


if(USE_INTEL_OPENCL)
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/normals.aocx)
		configure_file(normals.aocx normals.aocx COPYONLY)
	endif()
else()
	configure_file(normals.cl normals.cl COPYONLY)
	configure_file(knobs.h knobs.h COPYONLY)
endif(USE_INTEL_OPENCL)



include_directories(${CMAKE_CURRENT_SOURCE_DIR})

