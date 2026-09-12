// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
// Пустой TU: весь код libggcore приезжает из статических библиотек ядра
// через WHOLE_ARCHIVE (см. CMakeLists.txt рядом), но add_library(SHARED)
// требует хотя бы один исходник.
