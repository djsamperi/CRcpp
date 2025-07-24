cmake_minimum_required(VERSION 3.6)

# Uses CMake to build Rcpp and RInside libraries suitable for use
# with GCC (CRAN default for Windows), Visual Studio IDE,
# Ubuntu Linux (GCC), and MacOS (Clang, for example). An executable
# is also built that simply calls the REPL provided by RInside,
# useful for interacting with R while testing and debugging C++
# software.

# Tested using Visual Studio 2022 (Community Edition), GCC under
# Windows, GCC under Ubuntu Linux (with VS code), and 
# Clang on MacOS (with Xcode).

# CMake takes care of most of the system-dependent details, which
# explains the "C" in CRcpp.

project(CRcpp CXX)

# CMake Windows bug: When working in the Visual Studio CMD window, if
# we configure using 'cmake -G"MinGW Makefiles" ..`, MSVC is TRUE
# and cl.exe is used! Work-around: Use generic CMD window.
# message(STATUS "MSVC = ${MSVC}")

# Before we get started, check that the environment is configured
# properly, in particular, that R_LIBS has been set...
if(WIN32)
  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bin/checkwinpath.bat
    RESULT_VARIABLE ENV_PROBLEM
  )
else()
  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bin/checkunixpath.sh
    RESULT_VARIABLE ENV_PROBLEM
  )
endif()

if(ENV_PROBLEM)
  message(FATAL_ERROR "Terminating CMake (see message above)")
endif()

if(WIN32 AND R_LIBS_WIN)
  set(R_LIBS ${R_LIBS_WIN})
endif()

if(NOT R_LIBS) # Set on command-line or in .vscode/settings.json?
  set(R_LIBS $ENV{R_LIBS}) # Set in environment?
endif()
if("${R_LIBS}" STREQUAL "")
message(FATAL_ERROR
  "\n-- PROBLEM: Must set R_LIBS (library target directory path). "
  "Set this environment variable, and use R to install "
  "Rcpp and RInside packages into this user library before "
  "configuring and building CRcpp. "
  "This has the same effect as the R command "
  "\"install.packages(Rcpp, lib=<dir>)\", where \"<dir>\" is the "
  "first directory in R_LIBS. Rcpp shared library will be "
  "copied to \"<dir>/Rcpp/libs\" after CRcpp build, and similarly "
  "for RInside shared library. ")
endif()

# Fetch first directory in R_LIBS (user library). CRcpp will
# copy the output Rcpp and RInside libs over the
# ones that were installed by R.
if(WIN32)
  set(DELIMITER ";")
else()
  set(DELIMITER ":")
endif()
string(FIND "${R_LIBS}" "${DELIMITER}" LOC)
if(LOC GREATER 0)
  string(SUBSTRING "${R_LIBS}" 0 ${LOC} R_USER_LIB)
else()
  set(R_USER_LIB ${R_LIBS})
endif()
message(STATUS "Using R User Library ${R_USER_LIB}")

# Add CMakeLists.txt files for Rcpp and RInside, and also add
# Rcpp/src/CRcppBuildRcpp.cpp and RInside/src/CRcppBuildRInside.cpp
# (so shared libraries can be easily distinguished from the ones
# installed by R).
execute_process(
 COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/bin/copycmakefiles.sh 
            ${CMAKE_CURRENT_SOURCE_DIR}
)

# Patch Rcpp/RInside source for Microsoft Visual C++ (MSVC)
if(MSVC)
execute_process(
 COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/bin/msvcpatch.sh 
            ${CMAKE_CURRENT_SOURCE_DIR}
)
endif()

# The platform/architecture can be set here for Microsoft compilers. 
# Only the x64 architecture is supported under MSVC.
if(MSVC)
  # See comments in include/R_ext/Complex.h
  add_compile_definitions(R_LEGACY_RCOMPLEX=1)

  set(CMAKE_GENERATOR_PLATFORM x64)
  # set(CMAKE_GENERATOR_PLATFORM Win32)
  # Using Win32 would lead to undefined refs at link time, requiring 
  # some hacking to resolve. Since most projects use x64 today, 
  # this should not be a problem.
endif()

if(MSVC)
  # By default the startup project in Visual Studio is
  # ALL_BUILD. Change the startup project to CRcpp (REPL).
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
VS_STARTUP_PROJECT CRcpp)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
find_package(R)
# Set module search path, and use module FindR.cmake to find and
# configure linking to installed R platform. PATH must be set to
# find the desired version of R before running CMake.

if(WIN32)
  # Under Windows R_LIBRARY_BASE includes the library name at the end.
  # We strip it off to get R_LIBRARY_DIR...
  string(FIND "${R_LIBRARY_BASE}" "/" LOC REVERSE)
  string(SUBSTRING "${R_LIBRARY_BASE}" 0 ${LOC} SUBSTR)
  set(R_LIBRARY_DIR "${SUBSTR}")
else()
  set(R_LIBRARY_DIR "${R_HOME}/lib")
endif()

if(MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  # This eliminates the need to play with module definition files
  # in order to create a library interface file (.lib) for each dll.
  # It also eliminates the need to decorate source code with
  # __declspec(dllexport) and __declspec(dllimport).
endif()

# Default to using shared libraries. Static case has not been
# tested.
if(STATIC_LIBS)
set(BUILD_SHARED_LIBS FALSE)
else()
set(BUILD_SHARED_LIBS TRUE)
endif()

# Compiler options
if(MSVC)
  # Set warning level (off for now to suppress a flood of warnings).
  add_compile_options(/W0)
else()
  # For debugging with gdb
  add_compile_options(-g)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Rcpp/inst/include
  "${CMAKE_CURRENT_SOURCE_DIR}/RInside/inst/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/Rcpp/inst/include"
  ${R_INCLUDE_DIR})

if(WIN32)
  # R convention is to append /x64 to binary directory names, and
  # to use $R_HOME/bin/x64 instead of $R_HOME/lib used under UNIX.
  set(WIN_X64 "/x64")
  set(WIN_BINX64 "/bin/x64")
endif()

link_directories(
                 "${PROJECT_BINARY_DIR}"
                 "${R_LIBRARY_DIR}"
)

# Main app CRcpp has only one source file, repl.cpp.
# Add WIN32 to have Windows use WinMain entry point (GUI)
#add_executable(CRcpp WIN32 ${CMAKE_CURRENT_SOURCE_DIR}/src/repl.cpp)
add_executable(CRcpp ${CMAKE_CURRENT_SOURCE_DIR}/src/repl.cpp)

# Link CRcpp to R and RInside libs (Rcpp is header-only)
target_link_libraries(CRcpp R RInside)

# Setup custom Rcpp/RInside include directories
target_include_directories(CRcpp PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/RInside/inst/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Rcpp/inst/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${R_INCLUDE_DIR}"
    "${PROJECT_BINARY_DIR}")

# Specify where custom static and shared libs are built. Eliminates
# the need to explicitly modify PATH to point to dll's.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

# Add subproject builds for Rcpp and RInside
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Rcpp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/RInside)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Mypack)

# Optionally dump config information before build.
# Enable using: cmake -DSHOWCONF=TRUE ..
if(SHOW_CONF)
  message(STATUS "R_COMMAND: ${R_COMMAND}")
  message(STATUS "R_HOME: ${R_HOME}")
  message(STATUS "R_ROOT_DIR: ${R_ROOT_DIR}")
  message(STATUS "R_INCLUDE_DIR: ${R_INCLUDE_DIR}")
  message(STATUS "R_LIBRARY_DIR: ${R_LIBRARY_DIR}")
  message(STATUS "R_LIBRARY_BASE: ${R_LIBRARY_BASE}")
  message(STATUS "R_LIBRARY_BLAS: ${R_LIBRARY_BLAS}")
  message(STATUS "R_LIBRARY_GRAPHAPP: ${R_LIBRARY_GRAPHAPP}")
  message(STATUS "R_LIBRARY_LAPACK: ${R_LIBRARY_LAPACK}")
  message(STATUS "R_LIBRARY_READLINE: ${R_LIBRARY_READLINE}")
  message(STATUS "R_LIBS_USER: ${R_LIBS_USER}")
  message(STATUS "R_LIBRARIES: ${R_LIBRARIES}")
  message(STATUS "UNIX: ${UNIX}")
  message(STATUS "APPLE ${APPLE}")
  message(STATUS "WINDOWS: ${WIN32}")
  message(STATUS "MSVC: ${MSVC}")
  message(STATUS "MinGW: ${MINGW}")
  message(STATUS "CMake compiler: ${CMAKE_CXX_COMPILER_ID}")

  get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
  INCLUDE_DIRECTORIES)
  message(STATUS "Include dirs...")
  foreach(dir ${dirs})
    message(STATUS "  ${dir}")
  endforeach()

  get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
  message(STATUS "Library dirs...")
  foreach(dir ${dirs})
    message(STATUS "  ${dir}")
  endforeach()

endif()
