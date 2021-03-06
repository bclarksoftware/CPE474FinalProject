cmake_minimum_required(VERSION 2.8)

# Name of the project
project(FinalProject)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "src/*.cpp")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "src/*.hpp")
file(GLOB_RECURSE GLSL "resources/*.glsl" "resources/*.vs" "resources/*.fs")

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})

# Get the Eigen environment variable. Since Eigen is a header-only library, we
# just need to add it to the include directory.
set(EIGEN3_INCLUDE_DIR "$ENV{EIGEN3_INCLUDE_DIR}")
if(NOT EIGEN3_INCLUDE_DIR)
  MESSAGE(FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
endif()
include_directories(${EIGEN3_INCLUDE_DIR})

# Get the GLFW environment variable. There should be a CMakeLists.txt in the 
# specified directory.
set(GLFW_DIR "$ENV{GLFW_DIR}")
if(NOT GLFW_DIR)
  message(FATAL_ERROR "Please point the environment variable GLFW_DIR to the root directory of your GLFW3 installation.")
endif()
option(GLFW_BUILD_EXAMPLES "GLFW_BUILD_EXAMPLES" OFF)
option(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)
if(CMAKE_BUILD_TYPE MATCHES Release)
  add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/release)
else()
  add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/debug)
endif()
include_directories(${GLFW_DIR}/include)
target_link_libraries(${CMAKE_PROJECT_NAME} glfw ${GLFW_LIBRARIES})

# Get the GLEW environment variable.
set(GLEW_DIR "$ENV{GLEW_DIR}")
if(NOT GLEW_DIR)
  message(FATAL_ERROR "Please point the environment variable GLEW_DIR to the root directory of your GLEW installation.")
endif()
include_directories(${GLEW_DIR}/include)
if(WIN32)
  # With prebuilt binaries
  target_link_libraries(${CMAKE_PROJECT_NAME} ${GLEW_DIR}/lib/Release/Win32/glew32s.lib)
else()
  target_link_libraries(${CMAKE_PROJECT_NAME} ${GLEW_DIR}/lib/libGLEW.a)
endif()

# OS specific options and libraries
if(WIN32)
  # c++0x is enabled by default.
  # -Wall produces way too many warnings.
  # -pedantic is not supported.
else()
  # Enable all pedantic warnings.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic")
  if(APPLE)
    # Add required frameworks for GLFW.
    target_link_libraries(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
    #include_directories(${DEP_DIR}/FMOD/api/inc)
    #target_link_libraries(${CMAKE_PROJECT_NAME} ${DEP_DIR}/FMOD/api/lib/libfmodex.dylib)
  else()
    #Link the Linux OpenGL library
    target_link_libraries(${CMAKE_PROJECT_NAME} "GL")
  endif()
endif()

# Get the ASSIMP environment variable.
set(ASSIMP_DIR "$ENV{ASSIMP_DIR}")
if(NOT ASSIMP_DIR)
    message(FATAL_ERROR "Please point the environment variable ASSIMP_DIR to the root directory of your Assimp installation.")
endif()
include_directories(${ASSIMP_DIR}/include/assimp)
if(WIN32)
    # With prebuilt binaries
    #target_link_libraries(${CMAKE_PROJECT_NAME} ${GLEW_DIR}/lib/Release/Win32/glew32s.lib)
else()
    target_link_libraries(${CMAKE_PROJECT_NAME} ${ASSIMP_DIR}/lib/libassimp.dylib)
endif()

# Get the FMOD environment variable.
set(FMOD_DIR "$ENV{FMOD_DIR}")
if(NOT FMOD_DIR)
    message(FATAL_ERROR "Please point the environment variable FMOD_DIR to the root directory of your FMOD installation.")
endif()
include_directories(${FMOD_DIR}/api/inc)
if(WIN32)
    # With prebuilt binaries
    #target_link_libraries(${CMAKE_PROJECT_NAME} ${GLEW_DIR}/lib/Release/Win32/glew32s.lib)
else()
target_link_libraries(${CMAKE_PROJECT_NAME} ${FMOD_DIR}/api/lib/libfmodex.dylib)
endif()

# Get the Magick environment variable.
set(MAGICK_DIR "$ENV{MAGICK_DIR}")
if(NOT MAGICK_DIR)
	message(FATAL_ERROR "Please point the environment variable MAGICK_DIR to the root directory of your Magick installation.")
endif()
include_directories(${MAGICK_DIR}/include)
include_directories(${MAGICK_DIR}/include/ImageMagick-6)
if(WIN32)
	# With prebuilt binaries
	#target_link_libraries(${CMAKE_PROJECT_NAME} ${GLEW_DIR}/lib/Release/Win32/glew32s.lib)
else()
	target_link_libraries(${CMAKE_PROJECT_NAME} ${MAGICK_DIR}/lib/libMagick++-6.Q16.dylib)
endif()
