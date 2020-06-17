#include "gcutils.h"
#include <QFileInfo>
#include <app.h>
#include <project.h>
#include <settings.h>

namespace GCode {

const QVector<QChar> GCUtils::cmdList{ 'G', 'X', 'Y', 'Z', 'F', 'S' };
QString GCUtils::lastDir;

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
    qDebug() << QFileInfo(lastDir).absolutePath() << lastDir;
    return lastDir += '/';
}

void GCUtils::setLastDir(QString value)
{
    if (GlobalSettings::gcSameFolder())
        return;
    value = QFileInfo(value).absolutePath(); //value.left(value.lastIndexOf('/') + 1);
    if (lastDir != value) {
        lastDir = value;
        QSettings settings;
        settings.setValue("LastGCodeDir", lastDir);
    }
}

QVector<double> GCUtils::getDepths()
{
    const auto gDepth{ m_gcp.getDepth() };
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
