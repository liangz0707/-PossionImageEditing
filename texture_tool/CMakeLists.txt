cmake_minimum_required(VERSION 3.16)

project(texture_tool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
  tinyexr
  GIT_REPOSITORY https://github.com/syoyo/tinyexr.git
  GIT_TAG master
)
FetchContent_MakeAvailable(tinyexr)

FetchContent_Declare(
  qoi
  GIT_REPOSITORY https://github.com/phoboslab/qoi.git
  GIT_TAG master
)
FetchContent_GetProperties(qoi)
if(NOT qoi_POPULATED)
  FetchContent_Populate(qoi)
endif()

add_library(texture_lib SHARED
  src/Texture.cpp
  src/TextureIO.cpp
  src/stb_impl.cpp
  src/qoi_impl.cpp
  src/tinyexr_impl.cpp
)
target_include_directories(texture_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${tinyexr_SOURCE_DIR}
    ${qoi_SOURCE_DIR}
)
target_compile_definitions(texture_lib PRIVATE TEXTURE_EXPORTS TEXTURE_BUILD_SHARED)

option(TEXTURE_USE_OIIO "Enable OpenImageIO codec" ON)
if(TEXTURE_USE_OIIO)
  find_package(OpenImageIO QUIET)
  if(OpenImageIO_FOUND)
    target_compile_definitions(texture_lib PRIVATE TEXTURE_HAS_OIIO=1)
    target_link_libraries(texture_lib PRIVATE OpenImageIO::OpenImageIO)
  else()
    message(STATUS "OpenImageIO not found; OIIO codec disabled.")
  endif()
endif()

add_executable(texture_tool
  src/main.cpp
)
target_link_libraries(texture_tool PRIVATE texture_lib)
