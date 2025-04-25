/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "abstract_fileplugin.h"

namespace GCode {

class Tab : public AbstractFileSettings {
    QCheckBox* chbxInfo;
    QCheckBox* chbxSameGFolder;
    //    QCheckBox* chbxSimplifyHldi;
    //    QComboBox* cbxProfileSort;
    QLineEdit* leFileExtension;
    QLineEdit* leFormatMilling;
    QLineEdit* leFormatLaser;
    QLineEdit* leLaserCPC;
    QLineEdit* leLaserDPC;
    QLineEdit* leSpindleCC;
    QLineEdit* leSpindleLaserOff;
    QPlainTextEdit* pteEnd;
    QPlainTextEdit* pteLaserEnd;
    QPlainTextEdit* pteLaserStart;
    QPlainTextEdit* pteStart;
    QTabWidget* tabWidget;

public:
    Tab(QWidget* parent);
    virtual ~Tab() override;
    virtual void readSettings(MySettings& settings) override;
    virtual void writeSettings(MySettings& settings) override;
};

} // namespace GCode
