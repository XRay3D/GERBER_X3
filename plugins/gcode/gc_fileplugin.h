///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  March 25, 2023                                                  *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2023                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// ********************************************************************************/
// #pragma once

// #include "abstract_fileplugin.h"

// #include <QObject>

// namespace GCode {

// class Plugin : public AbstractFilePlugin {
//     Q_OBJECT

// public:
//     explicit Plugin(QObject* parent = nullptr);

//    bool thisIsIt(const QString& fileName) override;
//    uint32_t type() const override;
//    QString folderName() const override;

//    AbstractFileSettings* createSettingsTab(QWidget* parent) override;
//    AbstractFile* loadFile(QDataStream& stream) constoverride;

//    QIcon icon() const override;
//    void createMainMenu(QMenu& menu, FileTree::View* tv) override;

// public slots:
//     AbstractFile* parseFile(const QString& fileName, int type) override;
// };

//} // namespace GCode
