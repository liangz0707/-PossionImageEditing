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

add_library(texture_lib
  src/Texture.cpp
  src/TextureIO.cpp
  src/stb_impl.cpp
)
target_include_directories(texture_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
)

add_executable(texture_tool
  src/main.cpp
)
target_link_libraries(texture_tool PRIVATE texture_lib)
