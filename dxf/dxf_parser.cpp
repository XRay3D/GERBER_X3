// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_parser.h"
#include "dxf_file.h"

#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "section/dxf_headerparser.h"
#include "section/dxf_tables.h"
//#include "section/dxf_classes.h"
//#include "section/dxf_objects.h"
//#include "section/dxf_thumbnailimage.h"

#include "tables/dxf_layer.h"

namespace Dxf {

Parser::Parser(QObject* parent)
    : FileParser(parent)
{
}

AbstractFile* Parser::parseFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;

    m_file = new File;
    m_file->setFileName(fileName);

    int line = 1;

    Codes codes;
    codes.reserve(10000);

    QTextStream in(&file);

    auto getCode = [&in, &codes, &line, this] {
        // Code
        QString strCode(in.readLine());
        m_file->lines().append(strCode);
        bool ok;
        auto code(strCode.toInt(&ok));
        if (!ok)
            throw QString("Unknown code: raw str %1, line %2!").arg(strCode).arg(line);
        // Value
        QString strValue(in.readLine());
        m_file->lines().append(strValue);
        int multi = 0;
        while (strValue.endsWith("\\P")) {
            m_file->lines().append(in.readLine());
            strValue.append("\n" + m_file->lines().last());
            ++multi;
        }
        codes.emplace_back(code, strValue, line);
        line += 2 + multi;
        return *(codes.end() - 1);
    };

    try {
        int progress = 0;
        //int progressCtr = 0;
        do {
            if (auto code = getCode(); code.code() == 0 && code == "SECTION")
                ++progress;
        } while (!in.atEnd() || *(codes.end() - 1) != "EOF");
        codes.shrink_to_fit();

        //emit fileProgress(m_file->shortName(), progress, progressCtr);

        for (auto it = codes.begin(), from = codes.begin(), to = codes.begin(); it != codes.end(); ++it) {
            if (*it == "SECTION")
                from = it;
            if (auto it_ = it + 1; *it == "ENDSEC" && (*it_ == "SECTION" || *it_ == "EOF")) {
                //emit fileProgress(m_file->shortName(), 0, progressCtr++);
                to = it;
                const auto type = SectionParser::toType(*(from + 1));
                switch (type) {
                case SectionParser::HEADER:
                    dxfFile()->m_sections[type] = new SectionHEADER(dxfFile(), from, to);
                    break;
                case SectionParser::CLASSES:
                    //dxfFile()->m_sections[type] = new SectionCLASSES(dxfFile(), from, to);
                    break;
                case SectionParser::TABLES:
                    dxfFile()->m_sections[type] = new SectionTABLES(dxfFile(), from, to);
                    break;
                case SectionParser::BLOCKS:
                    dxfFile()->m_sections[type] = new SectionBLOCKS(dxfFile(), from, to);
                    break;
                case SectionParser::ENTITIES:
                    dxfFile()->m_sections[type] = new SectionENTITIES(dxfFile(), from, to);
                    break;
                case SectionParser::OBJECTS:
                    //dxfFile()->m_sections[type] = new SectionOBJECTS(dxfFile(), from, to);
                    break;
                case SectionParser::THUMBNAILIMAGE:
                    //dxfFile()->m_sections[type] = new SectionTHUMBNAILIMAGE(dxfFile(), from, to);
                    break;
                default:
                    throw QString("Unknowh Section!");
                    break;
                }
                if (dxfFile()->m_sections.contains(type))
                    dxfFile()->m_sections[type]->parse();
            }
        }
        file.close();
        if (dxfFile()->m_sections.size() == 0) {
            delete m_file;
            m_file = nullptr;
        } else {
            //emit fileProgress(m_file->shortName(), 1, 1);
            emit fileReady(m_file);
        }
    } catch (const QString& wath) {
        qWarning() << "exeption QString:" << wath;
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError( QFileInfo(fileName).fileName() , wath);
        delete m_file;
        return nullptr;
    } catch (const std::exception& e) {
        qWarning() << "exeption:" << e.what();
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError( QFileInfo(fileName).fileName() , "Unknown Error! " + QString(e.what()));
        delete m_file;
        return nullptr;
    } catch (...) {
        qWarning() << "exeption:" << errno;
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError( QFileInfo(fileName).fileName() , "Unknown Error! " + QString::number(errno));
        delete m_file;
        return nullptr;
    }
    return m_file;
}

bool Parser::isDxfFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;
    QTextStream in(&file);
    do {
        QString line(in.readLine());
        if (line.toInt() == 999) {
            line = in.readLine();
            line = in.readLine();
        }
        if (line.toInt() != 0)
            break;
        if (line = in.readLine(); line != "SECTION")
            break;
        if (line = in.readLine(); line.toInt() != 2)
            break;
        if (line = in.readLine(); line != "HEADER")
            break;
        return true;
    } while (false);
    return false;
}
}
