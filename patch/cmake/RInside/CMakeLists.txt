cmake_minimum_required(VERSION 3.18)

# Build RInside library and then copy it over the one that was
# installed by R into the user library specified by R_LIBS.
# The first directory in R_LIBS is used, and it must exist before
# CRcpp is configured.

project(RInside CXX)

# Generate extra headers needed to build RInside at config time.
execute_process(
     COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/../bin/rinsideheaders.sh ${CMAKE_CURRENT_SOURCE_DIR}/src
)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
                  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")


add_library(RInside ${SOURCES})

target_link_libraries(RInside Rcpp R)

if(WIN32)
  set(OUTPUT_LIB ${R_USER_LIB}/RInside/libs/x64/RInside.dll)
else()
  set(OUTPUT_LIB ${R_USER_LIB}/RInside/libs/RInside.so)
endif()

add_custom_command(TARGET RInside POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:RInside>
    ${OUTPUT_LIB}
  COMMAND echo "Copying $<TARGET_FILE:RInside> to ${OUTPUT_LIB}"
)

# Sign DLL for development.
if(APPLE)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND codesign -s - --force --timestamp=none ${OUTPUT_LIB}
)
endif()
