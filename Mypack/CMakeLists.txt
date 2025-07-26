cmake_minimum_required(VERSION 3.18)

# Use this CMakelists.txt build file to build shared libraries for
# your own R package that uses Rcpp. To get started you can use
# Rcpp.package.skeleton('Mypack') to insert the package source into
# CRcpp/Mypack. Replace Mypack with your own package name and update
# the project() line below accordingly.

# Insert an add_subdirectory() command in
# the root CMakelist.txt file for CRcpp so that your package is
# included in the build process (follow the pattern used for the
# existing add_subdirectory() commands in that file).

# After the library is built it will be copied over the one created by
# R under R_LIBS.

project(Mypack CXX)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
                  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")


add_library(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} R)

if(WIN32)
  set(OUTPUT_LIB ${R_USER_LIB}/${PROJECT_NAME}/libs/x64/${PROJECT_NAME}.dll)
else()
  set(OUTPUT_LIB ${R_USER_LIB}/${PROJECT_NAME}/libs/${PROJECT_NAME}.so)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${PROJECT_NAME}>
    ${OUTPUT_LIB}
  COMMAND echo "Copying $<TARGET_FILE:${PROJECT_NAME}> to ${OUTPUT_LIB}"
)

# Sign DLL for development.
if(APPLE)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND codesign -s - --force --timestamp=none ${OUTPUT_LIB}
)
endif()
