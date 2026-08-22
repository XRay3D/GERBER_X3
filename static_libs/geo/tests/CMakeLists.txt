# Тесты самой библиотеки -- только они: разбор DXF и GUI живут выше и
# проверяются своими (см. tests/ в корне).

find_package(Qt6 REQUIRED COMPONENTS Test)

foreach(name polyline util polygon inflate inflatepasses cancel normalize clipopen fill)
  add_executable(geo_test_${name} test_${name}.cpp)
  target_link_libraries(geo_test_${name} PRIVATE Geo::Geo Qt6::Test)
  add_test(NAME geo_${name} COMMAND geo_test_${name})
endforeach()

# Отмена меряет себя относительно полного счёта, поэтому идёт дольше
# прочих; на медленной машине запас лишним не будет.
set_tests_properties(geo_cancel PROPERTIES TIMEOUT 300)

# Бенчмарк кармана «снаружи» -- не тест: запускается руками для A/B-замеров
# (см. комментарий в bench_pocket.cpp), в ctest не регистрируется.
add_executable(geo_bench_pocket bench_pocket.cpp)
target_link_libraries(geo_bench_pocket PRIVATE Geo::Geo)
