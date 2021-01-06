SUFFIX = ""

contains(QT_ARCH, i386) {
    SUFFIX = "_x32"
} else {
    SUFFIX = "_x64"
}
msvc* {
    SUFFIX = $$SUFFIX"_msvc"
}
gcc* {
    SUFFIX = $$SUFFIX"_gcc"
}
CONFIG(debug, debug|release){
    SUFFIX = $$SUFFIX"_d"
}
