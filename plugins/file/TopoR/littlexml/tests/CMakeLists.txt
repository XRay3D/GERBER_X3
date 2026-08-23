add_executable(lxml_tests lxml_tests.cpp)
target_link_libraries(lxml_tests PRIVATE lxml::lxml)
target_compile_features(lxml_tests PRIVATE cxx_std_26)

# lxml.hpp тянет lxmlser.hpp (<meta>) -- нужна рефлексия.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(lxml_tests PRIVATE -freflection-latest
                                            -fexpansion-statements -stdlib=libc++)
  target_link_options(lxml_tests PRIVATE -fuse-ld=lld -lc++ -lc++abi)
else()
  # GR=1 -- как в plugins/file/TopoR/CMakeLists.txt: ветка lxmlser под GCC-рефлексию.
  target_compile_options(lxml_tests PRIVATE -freflection)
  target_compile_definitions(lxml_tests PRIVATE GR=1)
endif()

add_test(NAME lxml.parse COMMAND lxml_tests)
