cmake_minimum_required(VERSION 4.00)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(lxml LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 26) set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
# add_compile_options(-freflection-latest -fexpansion-statements -stdlib=libc++)
# add_link_options(-fuse-ld=lld  -lc++ -lc++abi) else()
# add_compile_options(-fmodules-ts) endif()

if(LXML_USE_MODULES)
  add_library(lxml)
  target_sources(lxml PUBLIC FILE_SET modules_public TYPE CXX_MODULES FILES
                             modules/lxml.cppm)

  target_compile_features(lxml PUBLIC cxx_std_26)
  target_compile_definitions(lxml PUBLIC LXML_USE_MODULES)

  if(CMAKE_CXX_COMPILER_IMPORT_STD)
    target_compile_definitions(lxml PRIVATE LXML_USE_STD_MODULE)
    message(STATUS "Using `import std;`")
  else()
    message(STATUS "`import std;` is not available")
  endif()
  target_include_directories(lxml PUBLIC include)
else()
  add_library(lxml INTERFACE)
  target_include_directories(lxml INTERFACE include)
endif()

add_library(lxml::lxml ALIAS lxml)

# Тесты: по умолчанию только при сборке littlexml отдельным проектом;
# в составе GERBER_X3 включаются через -DLXML_BUILD_TESTS=ON.
option(LXML_BUILD_TESTS "Build littlexml tests" ${PROJECT_IS_TOP_LEVEL})
if(LXML_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
