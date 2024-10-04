cmake_minimum_required(VERSION 3.1.0)

project (albums VERSION 0.1 LANGUAGES CXX)

# Default to Release build
if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif()

# Add a #define for Debug builds
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Build with C++ 17
set (CMAKE_CXX_STANDARD 17)

# AUTOMOC and AUTOUIC are both required, however, I may as well keep
# AUTORCC on too, just in case I want to use a resource file.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_library(libpqxx REQUIRED)
find_library(libpq REQUIRED)

# Documenation build
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Build the API docs" ${DOXYGEN_FOUND})
if (DOXYGEN_FOUND)
	set (DOXYGEN_INPUT Doxyfile)
endif (DOXYGEN_FOUND)

add_custom_target( docs
	command ${DOXYGEN_EXECUTABLE} Doxyfile
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	COMMENT "Generating API documentation."
	VERBATIM )

# Build the project
add_subdirectory(src)

