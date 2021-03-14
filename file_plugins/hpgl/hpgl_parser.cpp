// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "hpgl_parser.h"
#include "hpgl_file.h"

#include <QMetaEnum>

namespace Hpgl {

Parser::Cmd Parser::toCmd(const QStringRef& key)
{
    return static_cast<Cmd>(staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()));
}

Parser::Parser(FilePluginInterface* const interface)
    : interface(interface)
{
}

FileInterface* Parser::parseFile(const QString& fileName)
{
    QFile file_(fileName);
    if (!file_.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    file = new File;
    file->setFileName(fileName);

    QTextStream in(&file_);
    in.setAutoDetectUnicode(true);

    QString line;
    while (in.readLineInto(&line)) {
        switch (Cmd cmd = toCmd(line.midRef(0, 2)); cmd) {
        default:
            qDebug() << cmd;
        }
        //        file->lines().push_back(line);
        //        try {
        //            if (line == "%")
        //                continue;

        //            if (parseComment(line))
        //                continue;

        //            if (parseFormat(line))
        //                continue;

        //            if (parseTCode(line))
        //                continue;

        //            if (parseGCode(line))
        //                continue;

        //            if (parseMCode(line))
        //                continue;

        //            if (parseRepeat(line))
        //                continue;

        //            if (parseSlot(line))
        //                continue;

        //            if (parsePos(line))
        //                continue;
        //            qWarning() << "Excellon unparsed:" << line;
        //        } catch (const QString& errStr) {
        //            qWarning() << "exeption Q:" << errStr;
        //            emit interface->fileError("", QFileInfo(fileName).fileName() + "\n" + errStr);
        //            delete file;
        //            return nullptr;
        //        } catch (...) {
        //            qWarning() << "exeption S:" << errno;
        //            emit interface->fileError("", QFileInfo(fileName).fileName() + "\n" + "Unknown Error!");
        //            delete file;
        //            return nullptr;
        //        }
    }
    //    if (file->isEmpty()) {
    //        delete file;
    //        file = nullptr;
    //    } else {
    //        emit interface->fileReady(this->file);
    //    }
    return file;

    //    try {

    //    } catch (const QString& wath) {
    //        qWarning() << "exeption QString:" << wath;
    //        //emit fileProgress(m_file->shortName(), 1, 1);
    //        emit fileError(QFileInfo(fileName).fileName(), wath);
    //        delete file;
    //        return nullptr;
    //    } catch (const std::exception& e) {
    //        qWarning() << "exeption:" << e.what();
    //        //emit fileProgress(m_file->shortName(), 1, 1);
    //        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString(e.what()));
    //        delete file;
    //        return nullptr;
    //    } catch (...) {
    //        qWarning() << "exeption:" << errno;
    //        //emit fileProgress(m_file->shortName(), 1, 1);
    //        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString::number(errno));
    //        delete file;
    //        return nullptr;
    //    }
    //    return file;
}

}
