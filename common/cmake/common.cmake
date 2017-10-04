
# Use C++ 11
macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)


# Set a default build type
macro(set_default_build_type default_build_type)
	if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
	      STRING "Choose the type of build." FORCE)
	  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
	    "MinSizeRel" "RelWithDebInfo")
	endif()
endmacro(set_default_build_type)


# Setup OpenCL
macro(setup_intel_opencl TARGET)
	set(USE_INTEL_OPENCL OFF CACHE BOOL "Link against the Intel OpenCL libraries")

	if(USE_INTEL_OPENCL)

		execute_process(
			OUTPUT_VARIABLE AOCL_COMPILE_FLAGS
			COMMAND aocl compile-config
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		execute_process(
			OUTPUT_VARIABLE AOCL_LINK_FLAGS
			COMMAND aocl link-config
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		execute_process(
			OUTPUT_VARIABLE AOCL_VERSION
			COMMAND aocl version
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		if(${AOCL_VERSION} MATCHES "14.1")
			#string(REPLACE "lterasic" "laltera" AOCL_LINK_FLAGS ${AOCL_LINK_FLAGS})
			target_link_libraries(${TARGET} OpenCL dl)
		endif()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${AOCL_COMPILE_FLAGS}")
		set_target_properties(${TARGET} PROPERTIES LINK_FLAGS ${AOCL_LINK_FLAGS})
		
	else()
		find_package(OpenCL REQUIRED)
		target_link_libraries(${TARGET} ${OpenCL_LIBRARIES})
		include_directories(${OpenCL_INCLUDE_DIR})
	endif(USE_INTEL_OPENCL)

endmacro(setup_intel_opencl)

