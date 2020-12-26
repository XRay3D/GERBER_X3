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

#include "abstractfile.h"

#include "splashscreen.h"
#include <QApplication>

#include "leakdetector.h"

AbstractFile::AbstractFile()
    : m_itemGroups(1, new ItemGroup)
{
}

AbstractFile::~AbstractFile() { qDeleteAll(m_itemGroups); }

QString AbstractFile::shortName() const { return QFileInfo(m_name).fileName(); }

QString AbstractFile::name() const { return m_name; }

void AbstractFile::setFileName(const QString& fileName) { m_name = fileName; }

Paths AbstractFile::mergedPaths() const { return m_mergedPaths.size() ? m_mergedPaths : merge(); }

Pathss AbstractFile::groupedPaths() const { return m_groupedPaths; }

QList<QString>& AbstractFile::lines() { return m_lines; }

const LayerTypes& AbstractFile::displayedTypes() const { return m_layerTypes; }

Side AbstractFile::side() const { return m_side; }

void AbstractFile::setSide(Side side) { m_side = side; }

const QColor& AbstractFile::color() const { return m_color; }

void AbstractFile::setColor(const QColor& color) { m_color = color; }

int AbstractFile::id() const { return m_id; }

void AbstractFile::setId(int id) { m_id = id; }
