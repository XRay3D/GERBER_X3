#include "doublespinbox.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QFont f;
    f.setPixelSize(100);
    a.setFont(f);
    DoubleSpinBox w;
    w.resize(500, 100);
    w.setRange(-1000'000, +1000'000);
    w.setSuffix(" mm");
    w.show();
    return a.exec();
}
