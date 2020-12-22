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
#include "dxfparser.h"
#include "dxffile.h"

#include "section/blocks.h"
#include "section/classes.h"
#include "section/entities.h"
#include "section/headerparser.h"
#include "section/objects.h"
#include "section/sectionparser.h"
#include "section/tables.h"
#include "section/thumbnailimage.h"

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

    QTextStream in(&file);

    m_file = new File;
    m_file->setFileName(fileName);

    int ctr = 0;
    QVector<CodeData> codes;
    codes.reserve(1000000);

    try {
        do {
            codes.append({ in.readLine().toInt(), in.readLine(), ctr });
            ctr += 2;
            if (codes.last() == "ENDSEC") {
                int bsec = 0;
                while (codes[bsec++] != "SECTION" && bsec < codes.size())
                    continue;
                switch (SectionParser::key(QString(codes[bsec]))) {
                case SectionParser::HEADER:
                    dxfFile()->sections << new SectionHEADER(dxfFile(), std::move(codes));
                    break;
                case SectionParser::CLASSES:
                    dxfFile()->sections << new SectionCLASSES(dxfFile(), std::move(codes));
                    break;
                case SectionParser::TABLES:
                    dxfFile()->sections << new SectionTABLES(dxfFile(), std::move(codes));
                    break;
                case SectionParser::BLOCKS:
                    dxfFile()->sections << new SectionBLOCKS(dxfFile(), std::move(codes));
                    break;
                case SectionParser::ENTITIES:
                    dxfFile()->sections << new SectionENTITIES(dxfFile(), std::move(codes));
                    break;
                case SectionParser::OBJECTS:
                    dxfFile()->sections << new SectionOBJECTS(dxfFile(), std::move(codes));
                    break;
                case SectionParser::THUMBNAILIMAGE:
                    dxfFile()->sections << new SectionTHUMBNAILIMAGE(dxfFile(), std::move(codes));
                    break;
                default:
                    //throw codes.last();
                    break;
                }
                if (dxfFile()->sections.size())
                    dxfFile()->sections.last()->parse();
            }
        } while (!in.atEnd() || codes.last() != "EOF");
        codes.shrink_to_fit();
        file.close();
    } catch (...) {
        qWarning() << "exeption S:" << errno;
        emit fileError("", QFileInfo(fileName).fileName() + "\n" + "Unknown Error!");
        delete m_file;
        return nullptr;
    }

    if (dxfFile()->sections.isEmpty()) {
        delete m_file;
        m_file = nullptr;
    } else {
        emit fileReady(m_file);
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
        if (QString line(in.readLine()); line.toInt() != 0)
            break;
        if (QString line(in.readLine()); line != "SECTION")
            break;
        if (QString line(in.readLine()); line.toInt() != 2)
            break;
        if (QString line(in.readLine()); line != "HEADER")
            break;
        return true;
    } while (false);
    return false;
}

}
