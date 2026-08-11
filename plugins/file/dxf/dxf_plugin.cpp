/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_plugin.h"
#include "dxf_file.h"
#include "dxf_node.h"
#include "dxf_settingstab.h"

#include "geo/cancel.h"

#include "entities/dxf_circle.h"
#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "section/dxf_headerparser.h"
#include "section/dxf_tables.h"

#include "tables/dxf_layer.h"

#include "drill/drill_form.h"

#include <QtWidgets>

namespace Dxf {

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin{parent} {
}

AbstractFile* Plugin::parseFile(const QString& fileName, uint32_t type_) {
    if(type_ != type()) return nullptr;
    QFile file{fileName};
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << file.errorString();
        // Строку в окне прогресса завели ещё в MainWindow::loadFile, и снять её
        // может только сам плагин -- иначе она висит до конца сеанса.
        emit fileProgress(fileName, 1, 1);
        return nullptr;
    }

    file_ = new File;
    file_->setFileName(fileName);

    int line = 1;

    Codes codes;
    codes.reserve(10000);

    QByteArray data = file.readAll();
    file.close();

    detectEncoding(data);
    if(!isValidUtf8(data))
        data = toQString(data).toUtf8();

    QTextStream in{data /*&file*/};

    in.setAutoDetectUnicode(true); // BOM  ???

    // Ключ прогресса -- ПОЛНЫЙ путь: по нему окно находит свою строку, а
    // короткое имя не уникально (два detail.dxf из разных папок делили бы одну).
    emit fileProgress(fileName, int(data.count('\n')) + 1, 0);

    auto getCode = [&in, &codes, &line, this] {
        // Code
        QString strCode(in.readLine());
        file_->lines().push_back(strCode);
        bool ok;
        auto code(strCode.toInt(&ok));
        if(!ok)
            throw u"Unknown code: raw str %1, line %2!"_s.arg(strCode).arg(line);
        // Value
        QString strValue(in.readLine());
        file_->lines().push_back(strValue);
        int multi{};
        while(strValue.endsWith(u"\\P"_s)) {
            file_->lines().emplace_back(in.readLine());
            strValue.append(u'\n' + file_->lines().back());
            ++multi;
        }
        codes.emplace_back(code, strValue, line);
        line += 2 + multi;
        return *(codes.end() - 1);
    };

    try {
        int ctr{};
        do {
            getCode();
            // Считаем ПАРЫ (код + значение), а меряем строками: line растёт на
            // 2 и больше за пару, так что делить на порог надо счётчик пар.
            if(!(++ctr % 500)) {
                emit fileProgress(fileName, 0, line);
                // Отклик на кнопку отмены -- та же порция, что и у прогресса:
                // чаще незачем, а проверка не бесплатна.
                Geo::checkCancelled();
            }
        } while(!in.atEnd() || *(codes.end() - 1) != u"EOF"_s);
        codes.shrink_to_fit();

        Timer t{"Section Parser"};

        for(auto it = codes.begin(), from = codes.begin(), to = codes.begin(); it != codes.end(); ++it) {
            if(*it == u"SECTION"_s) from = it;
            if(auto it_ = it + 1; *it == u"ENDSEC"_s && (*it_ == u"SECTION"_s || *it_ == u"EOF"_s)) {
                // emit fileProgress(file_->shortName(), 0, progressCtr++);
                to = it;
                // const auto type = SectionParser::toType(*(from + 1)); // FIXME
                auto code = (from + 1)->string();
                const auto type = SectionParser::toType(code);
                switch(type) {
                case SectionParser::HEADER:
                    file_->sections_[type] = new SectionHEADER{file_, from, to};
                    continue;
                case SectionParser::CLASSES:
                    // dxfFile()->sections_[type] = new SectionCLASSES{dxfFile(), from, to};
                    continue;
                case SectionParser::TABLES:
                    file_->sections_[type] = new SectionTABLES{file_, from, to};
                    continue;
                case SectionParser::BLOCKS:
                    file_->sections_[type] = new SectionBLOCKS{file_, from, to};
                    continue;
                case SectionParser::ENTITIES:
                    file_->sections_[type] = new SectionENTITIES{file_, from, to};
                    continue;
                case SectionParser::OBJECTS:
                    // dxfFile()->sections_[type] = new SectionOBJECTS{dxfFile(), from, to};
                    continue;
                case SectionParser::THUMBNAILIMAGE:
                    // dxfFile()->sections_[type] = new SectionTHUMBNAILIMAGE{dxfFile(), from, to};
                    continue;
                default: throw u"Unknowh Section!"_s;
                }
            }
        }
        if(file_->sections_.size() == 0) {
            delete file_;
            file_ = nullptr;
            emit fileError(QFileInfo(fileName).fileName(), tr("No sections found!"));
            emit fileProgress(fileName, 1, 1);
        } else {
            // Проекционные слои строятся точной геометрией и на крупной сети
            // считаются долго -- отмена внутрь них уходит сама, областью
            // Geo::CancelScope, заведённой в AbstractFilePlugin::parseFileTask.
            file_->createProjectionLayers();
            emit fileReady(file_);
            emit fileProgress(fileName, 1, 1);
        }
    } catch(const Geo::Cancelled&) {
        // Отмена пользователем -- не ошибка: ни в лог, ни в диалог ошибок.
        // Сигналы шлёт AbstractFilePlugin::parseFileTask, туда и пробрасываем.
        // Ловим ДО std::exception: Cancelled от него наследуется и иначе уехал
        // бы в общий обработчик ошибок.
        delete file_;
        file_ = nullptr;
        throw;
    } catch(const QString& wath) {
        qWarning() << u"exeption QString:"_s << wath;
        emit fileError(QFileInfo(fileName).fileName(), wath);
        emit fileProgress(fileName, 1, 1);
        delete file_;
        file_ = nullptr;
        return nullptr;
    } catch(const std::exception& e) {
        qWarning() << u"exeption:"_s << e.what();
        emit fileError(QFileInfo(fileName).fileName(), u"Unknown Error! "_s + QString::fromUtf8(e.what()));
        emit fileProgress(fileName, 1, 1);
        delete file_;
        file_ = nullptr;
        return nullptr;
    } catch(...) {
        qWarning() << u"exeption:"_s << errno;
        emit fileError(QFileInfo(fileName).fileName(), u"Unknown Error! "_s + QString::number(errno));
        emit fileProgress(fileName, 1, 1);
        delete file_;
        file_ = nullptr;
        return nullptr;
    }
    return file_;
}

bool Plugin::thisIsIt(const QString& fileName) {

    if(fileName.endsWith(u".dxf"_s, Qt::CaseInsensitive))
        return true;

    QFile file{fileName};
    if(!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in{&file};
    do {
        QString line(in.readLine());
        if(line.toInt() == 999) {
            line = in.readLine();
            line = in.readLine();
        }
        if(line.toInt() != 0)
            break;
        if(line = in.readLine(); line != u"SECTION"_s)
            break;
        if(line = in.readLine(); line.toInt() != 2)
            break;
        if(line = in.readLine(); line != u"HEADER"_s)
            break;
        return true;
    } while(false);
    return false;
}

uint32_t Plugin::type() const { return DXF; }

AbstractFile* Plugin::loadFile(std::string_view json) const { return Serial::load<File>(json); }

// Имя типа — из аннотации на классе File: один источник истины.
std::string_view Plugin::typeName() const { return Serial::typeNameOf<File>(); }

QIcon Plugin::icon() const { return decoration(Qt::lightGray, u'D'); }

AbstractFileSettings* Plugin::createSettingsTab(QWidget* parent) {
    auto settingsTab = new SettingsTab{parent};
    settingsTab->setWindowTitle(u"DXF"_s);
    return settingsTab;
}

void Plugin::updateFileModel(AbstractFile* file) {
    const auto fm = App::fileModelPtr();
    const QModelIndex& fileIndex(file->node()->index());
    const QModelIndex index = fm->createIndex(0, 0, fileIndex.internalId());
    // clean before insert new layers
    if(int count = fm->getItem(fileIndex)->childCount(); count) {
        fm->beginRemoveRows(index, 0, count - 1);
        auto item = fm->getItem(index);
        do {
            item->remove(--count);
        } while(count);
        fm->endRemoveRows();
    }
    Dxf::Layers layers;

    for(auto& [name, layer]: static_cast<File*>(file)->layers())
        if(!layer->isEmpty()) layers[name] = layer;
    // Пустой файл (секции есть, рисовать нечего) даёт size() - 1 == -1, и
    // beginInsertRows на таком диапазоне -- обращение к модели с мусором.
    if(layers.empty()) return;
    fm->beginInsertRows(index, 0, int(layers.size()) - 1);

    for(auto& [name, layer]: layers)
        fm->getItem(index)->addChild(new Dxf::NodeLayer{name, layer});
    fm->endInsertRows();
}

} // namespace Dxf

#include "moc_dxf_plugin.cpp"
