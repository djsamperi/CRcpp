cmake_minimum_required(VERSION 3.6)

# Build Rcpp library and then copy it over the one that was
# installed by R into the user library specified by R_LIBS.
# The first directory in R_LIBS is used, and it must exist before
# CRcpp is configured.

project(Rcpp CXX)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
                  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

add_library(Rcpp ${SOURCES})

target_link_libraries(Rcpp R)

if(WIN32)
  set(OUTPUT_LIB ${R_USER_LIB}/Rcpp/libs/x64/Rcpp.dll)
else()
  # Both Linux and MacOS use .so suffix for shared library.
  set(OUTPUT_LIB ${R_USER_LIB}/Rcpp/libs/Rcpp.so)
endif()

# Copy (or link) Rcpp.so under R_LIBS to IDE version.
if(WIN32)
  add_custom_command(TARGET Rcpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      $<TARGET_FILE:Rcpp>
      ${OUTPUT_LIB}
    COMMAND echo "Copying $<TARGET_FILE:Rcpp> to ${OUTPUT_LIB}")
else()
  add_custom_command(TARGET Rcpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
      $<TARGET_FILE:Rcpp>
      ${OUTPUT_LIB}
    COMMAND echo "Linking $<TARGET_FILE:Rcpp> to ${OUTPUT_LIB}")
endif()


