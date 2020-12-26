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
#include "gcutils.h"
#include "app.h"
#include "math.h"
#include "project.h"
#include "settings.h"
#include <QFileInfo>

namespace GCode {

GCUtils::GCUtils(const GCodeParams& gcp)
    : m_gcp(gcp)
{
}

QString GCUtils::getLastDir()
{
    if (GlobalSettings::gcSameFolder())
        lastDir = QFileInfo(App::project()->name()).absolutePath();
    else if (lastDir.isEmpty()) {
        QSettings settings;
        lastDir = settings.value("LastGCodeDir").toString();
        if (lastDir.isEmpty())
            lastDir = QFileInfo(App::project()->name()).absolutePath();
        settings.setValue("LastGCodeDir", lastDir);
    }

    return lastDir += '/';
}

void GCUtils::setLastDir(QString value)
{
    if (GlobalSettings::gcSameFolder())
        return;
    value = QFileInfo(value).absolutePath();
    if (lastDir != value) {
        lastDir = value;
        QSettings settings;
        settings.setValue("LastGCodeDir", lastDir);
    }
}

QVector<double> GCUtils::getDepths()
{
    const auto gDepth { m_gcp.getDepth() };
    if (gDepth < m_gcp.getTool().passDepth() || qFuzzyCompare(gDepth, m_gcp.getTool().passDepth()))
        return { -gDepth - m_gcp.getTool().getDepth() };

    const int count = static_cast<int>(ceil(gDepth / m_gcp.getTool().passDepth()));
    const double depth = gDepth / count;
    QVector<double> depths(count);
    for (int i = 0; i < count; ++i)
        depths[i] = (i + 1) * -depth;
    depths.last() = -gDepth - m_gcp.getTool().getDepth();
    return depths;
}
}
