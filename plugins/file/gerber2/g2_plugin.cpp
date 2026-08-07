/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "g2_plugin.h"

#include "doublespinbox.h"
#include "settings.h"

#include <QtWidgets>

namespace Gerber2 {

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin{parent} { }

QIcon Plugin::icon() const { return decoration(Qt::darkCyan, u'2'); }

AbstractFile* Plugin::loadFile(QDataStream& stream) const { return AbstractFile::load<File>(stream); }

namespace {

bool preferred() {
    MySettings settings;
    settings.beginGroup(u"Gerber2"_s);
    return settings.value(u"chbxPreferred"_s, false).toBool();
}

bool isGerber(const QString& fileName) {
    static const QRegularExpression re{uR"(%FS[LTD]?[AI]X\d{2}Y\d{2}\*)"_s};
    QFile file{fileName};
    if(!file.open(QFile::ReadOnly | QFile::Text)) return false;
    QTextStream in{&file};
    QString line;
    while(in.readLineInto(&line))
        if(re.match(line).hasMatch()) return true;
    return false;
}

} // namespace

bool Plugin::thisIsIt(const QString& fileName) {
    if(!isGerber(fileName)) return false;
    // Оба гербер-плагина читают один формат, а хост отдаёт файл первому
    // откликнувшемуся. Если основной плагин собран - уступаем ему, пока
    // пользователь не выберет этот флажком в настройках.
    return preferred() || !App::filePlugin("Gerber"_hash32);
}

AbstractFileSettings* Plugin::createSettingsTab(QWidget* parent) {
    class Tab : public AbstractFileSettings {
        QCheckBox* chbxPreferred;

    public:
        explicit Tab(QWidget* parent)
            : AbstractFileSettings{parent} {
            setObjectName(u"tabGerber2"_s);

            chbxPreferred = new QCheckBox{tr("Open Gerber files with this plugin"), this};
            chbxPreferred->setObjectName(u"chbxPreferred"_s);
            chbxPreferred->setToolTip(tr("Otherwise Gerber files are handled by the main Gerber X3 plugin.\n"
                                         "Takes effect for files opened after the change."));

            auto layout = new QVBoxLayout{this};
            layout->setContentsMargins(6, 6, 6, 6);
            layout->addWidget(chbxPreferred);
            layout->addItem(new QSpacerItem{20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding});
        }
        void readSettings(MySettings& settings) override {
            settings.beginGroup(u"Gerber2"_s);
            settings.getValue(chbxPreferred, false);
            settings.endGroup();
        }
        void writeSettings(MySettings& settings) override {
            settings.beginGroup(u"Gerber2"_s);
            settings.setValue(chbxPreferred);
            settings.endGroup();
        }
    };
    auto tab = new Tab{parent};
    tab->setWindowTitle(u"Gerber X3 (editable)"_s);
    return tab;
}

AbstractFile* Plugin::parseFile(const QString& fileName, uint32_t type_) {
    if(type_ != type()) return nullptr;

    QFile source{fileName};
    if(!source.open(QFile::ReadOnly | QFile::Text)) {
        emit fileError(fileName, source.errorString());
        return nullptr;
    }
    QTextStream in{&source};
    in.setAutoDetectUnicode(true);

    auto file = std::make_unique<File>();
    file->setFileName(fileName);

    QString error;
    if(!file->setSource(in.readAll(), &error)) {
        emit fileError(fileName, error);
        return nullptr;
    }
    for(const QString& warning: file->parsed().warnings)
        emit fileWarning(fileName, warning);

    // parseFile вызывается через QueuedConnection - возвращаемое значение
    // теряется, файл попадает в проект только через этот сигнал.
    auto* result = file.release();
    emit fileReady(result);
    return result;
}

} // namespace Gerber2

#include "moc_g2_plugin.cpp"
