project(swig)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Remove '-Werror', '-Wall' and 'pedantic' flags (if present) when compiling SWIG-generated files
string(REGEX REPLACE "-Wall" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "-Wall" " " CMAKE_CXX_FLAGS_DEVELOPER "${CMAKE_CXX_FLAGS_DEVELOPER}")
string(REGEX REPLACE "-Werror=format-security" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "-Werror=format-security" " " CMAKE_CXX_FLAGS_DEVELOPER "${CMAKE_CXX_FLAGS_DEVELOPER}")
string(REGEX REPLACE "-Werror" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "-Werror" " " CMAKE_CXX_FLAGS_DEVELOPER "${CMAKE_CXX_FLAGS_DEVELOPER}")
string(REGEX REPLACE "-pedantic" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "-pedantic" " " CMAKE_CXX_FLAGS_DEVELOPER "${CMAKE_CXX_FLAGS_DEVELOPER}")

# Prevent Intel compiler emitting warnings for SWIG-generated code
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  if("${CMAKE_CXX_COMPILER_VERSION}" LESS "13")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd186")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -wd186")
    set(CMAKE_CXX_FLAGS_DEVELOPER "${CMAKE_CXX_FLAGS_DEVELOPER} -wd186")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -wd186")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -wd186")
  endif()
endif()

# Set common include directoris
include_directories(${PYTHON_INCLUDE_DIRS} ${NUMPY_INCLUDE_DIR})

# Set numpy version define
set(DOLFIN_PYTHON_DEFINITIONS -DNUMPY_VERSION_MAJOR=${NUMPY_VERSION_MAJOR} -DNUMPY_VERSION_MINOR=${NUMPY_VERSION_MINOR} -DNUMPY_VERSION_MICRO=${NUMPY_VERSION_MICRO})

# Only set define for none depricated API for NUMPY version 1.7 and larger
if(NUMPY_VERSION VERSION_GREATER 1.6.2)
  set(DOLFIN_PYTHON_DEFINITIONS ${DOLFIN_PYTHON_DEFINITIONS} -DNPY_NO_DEPRECATED_API=NPY_${NUMPY_VERSION_MAJOR}_${NUMPY_VERSION_MINOR}_API_VERSION)
endif()

add_definitions(${DOLFIN_PYTHON_DEFINITIONS})

# Get combined modules
file(GLOB COMBINED_MODULES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/modules/*)

# Sort and reverse the list so la combined module comes early
list(SORT COMBINED_MODULES)
list(REVERSE COMBINED_MODULES)

# Iterate over the combined modules and add each as a target
foreach(COMBINED_MODULE_DIR ${COMBINED_MODULES})
  add_subdirectory(${COMBINED_MODULE_DIR})
endforeach()

# List of files which should not be installed
set(NOT_INSTALL )

# Iterate over each sub module and add install targets
foreach(MODULE_DIR ${DOLFIN_DIRS})

  # Install pre.i if exists
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_DIR}/pre.i)
    install(FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_DIR}/pre.i
      DESTINATION
      ${DOLFIN_INCLUDE_DIR}/dolfin/swig/${MODULE_DIR}
      COMPONENT Development)
  endif()

endforeach()

# Add global interface files
file(GLOB GLOBAL_INTERFACE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.i)
install(FILES ${GLOBAL_INTERFACE_FILES}
  DESTINATION ${DOLFIN_INCLUDE_DIR}/dolfin/swig
  COMPONENT Development
  )

# Add global typemap interface files
file(GLOB TYPEMAP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/typemaps/*.i)
install(FILES ${TYPEMAP_FILES}
  DESTINATION ${DOLFIN_INCLUDE_DIR}/dolfin/swig/typemaps
  COMPONENT Development
  )
