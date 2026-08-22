// Лимбы GMP -- через mimalloc. Каждая операция EPECK молотит выделениями
// коротких лимбов из многих потоков разом, а malloc из ucrt под такой
// конкуренцией складывается -- та самая «стена GMP-аллокатора», о которую
// разбился эксперимент с параллельными витками (см. комментарий в
// pocketoffset.cpp). У mimalloc кучи по-поточные, и стены нет.
//
// Подмена точечная -- три указателя mp_set_memory_functions, а НЕ глобальный
// перехват new/malloc: перехват на MinGW ненадёжен, и блок, рождённый одним
// аллокатором в одном DLL и убитый другим в соседнем, роняет процесс. GMP же
// прогоняет через эти указатели каждый лимб (mpfr наследует их сам).
//
// Блоки, выделенные ДО переключения, живы: статики чужих модулей могли
// завести GMP-числа раньше нашего инициализатора. Свой блок узнаётся по
// магии в служебном заголовке перед данными; чужой уходит в free/realloc из
// ucrt, которыми и был выделен. Заголовок в 16 байт сохраняет выравнивание
// max_align_t для лимбов.

#include <gmp.h>
#include <mimalloc.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace Geo {

namespace {

constexpr std::uint64_t magic = 0x4D69476D70426C6Bull; // "MiGmpBlk"
constexpr std::size_t header = 16;

void* tag(void* base) {
    std::memcpy(base, &magic, sizeof magic);
    return static_cast<char*>(base) + header;
}

bool tagged(const void* p) {
    std::uint64_t m;
    std::memcpy(&m, static_cast<const char*>(p) - header, sizeof m);
    return m == magic;
}

void* gmpAlloc(std::size_t size) {
    // GMP на нехватку памяти сам зовёт abort -- нулю отсюда выйти некуда.
    void* base = mi_malloc(size + header);
    if(!base) std::abort();
    return tag(base);
}

void* gmpRealloc(void* p, std::size_t oldSize, std::size_t newSize) {
    if(tagged(p)) {
        void* base = mi_realloc(static_cast<char*>(p) - header, newSize + header);
        if(!base) std::abort();
        return static_cast<char*>(base) + header; // магия переехала вместе с данными
    }
    // Чужой блок: перекладывается в свой -- заодно и последующие realloc
    // этого числа пойдут уже через mimalloc.
    void* fresh = gmpAlloc(newSize);
    std::memcpy(fresh, p, std::min(oldSize, newSize));
    std::free(p);
    return fresh;
}

void gmpFree(void* p, std::size_t) {
    if(tagged(p)) mi_free(static_cast<char*>(p) - header);
    else std::free(p);
}

} // namespace

// Зовётся из статического инициализатора в cgal.cpp: до main и до первого
// точного числа Geo, пока процесс ещё однопоточен.
bool installGmpMiAlloc() {
    mp_set_memory_functions(gmpAlloc, gmpRealloc, gmpFree);
    return true;
}

} // namespace Geo
