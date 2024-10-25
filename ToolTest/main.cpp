// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "app.h"
#include "tool_database.h"

#include <QApplication>
#include <QStandardPaths>
#include <QtWidgets>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    QApplication::setApplicationName("GGEasy");
    //    QApplication::setOrganizationName(VER_COMPANYNAME_STR);
    //    QApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

    [[maybe_unused]] App appSingleton;
    App::settingsPath() = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).front();

    ToolDatabase w;
    w.show();

    emit w.findChild<QPushButton*>("pbNew")->clicked();

    return a.exec();
}
