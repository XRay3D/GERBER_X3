// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "gc_drillmodel.h"
#include "gc_drillform.h"
#include "tool_pch.h"

#include <QBitmap>
#include <QDebug>
#include <QPainter>

DrillModel::DrillModel(int type, int rowCount, QObject* parent)
    : QAbstractTableModel(parent)
    , m_type(type) {
    m_data.reserve(rowCount);
}

DrillModel::DrillModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

//Row& DrillModel::appendRow(const QString& name, const QIcon& icon, int id)
//{
//    m_data.emplace_back(name, icon, id);
//    return m_data.back();
//}

void DrillModel::setToolId(int row, int id) {
    if (m_data[row].toolId != id)
        m_data[row].useForCalc = id > -1;
    m_data[row].toolId = id;
    emit set(row, id > -1);
    emit dataChanged(createIndex(row, 0), createIndex(row, 1));
}

int DrillModel::toolId(int row) { return m_data[row].toolId; }

void DrillModel::setSlot(int row, bool slot) { m_data[row].isSlot = slot; }

bool DrillModel::isSlot(int row) { return m_data[row].isSlot; }

int DrillModel::apertureId(int row) { return m_data[row].apertureId; }

bool DrillModel::useForCalc(int row) const { return m_data[row].useForCalc; }

void DrillModel::setCreate(int row, bool create) {
    if (m_data[row].toolId == -1)
        return;
    m_data[row].useForCalc = create;
    emit dataChanged(createIndex(row, 0), createIndex(row, 1));
    emit headerDataChanged(Qt::Vertical, row, row);
}

void DrillModel::setCreate(bool create) {
    for (int row = 0; row < rowCount(); ++row) {
        m_data[row].useForCalc = create && m_data[row].toolId != -1;
    }
    emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 1));
}

int DrillModel::rowCount(const QModelIndex& /*parent*/) const { return static_cast<int>(m_data.size()); }

int DrillModel::columnCount(const QModelIndex& /*parent*/) const { return ColumnCount; }

QVariant DrillModel::data(const QModelIndex& index, int role) const {
    int row = index.row();
    if (index.column() == Name) {
        switch (role) {
        case Qt::DisplayRole:
            if (m_data[row].isSlot)
                return QString(m_data[row].name).replace(tr("Tool"), tr("Slot"));
            else
                return m_data[row].name;
        case Qt::DecorationRole: {
            if (m_data[index.row()].toolId > -1 && m_data[row].isSlot) {
                QImage image(m_data[row].icon.pixmap(24, 24).toImage());
                for (int x = 0; x < 24; ++x)
                    for (int y = 0; y < 24; ++y)
                        image.setPixelColor(x, y, QColor(255, 0, 0, image.pixelColor(x, y).alpha()));
                return QIcon(QPixmap::fromImage(image));
            } else if (m_data[index.row()].toolId > -1) {
                return m_data[row].icon;
            } else if (m_data[row].isSlot) {
                QImage image(m_data[row].icon.pixmap(24, 24).toImage());
                for (int x = 0; x < 24; ++x)
                    for (int y = 0; y < 24; ++y)
                        image.setPixelColor(x, y, QColor(255, 100, 100, image.pixelColor(x, y).alpha()));
                return QIcon(QPixmap::fromImage(image));
            } else {
                QImage image(m_data[row].icon.pixmap(24, 24).toImage());
                for (int x = 0; x < 24; ++x)
                    for (int y = 0; y < 24; ++y)
                        image.setPixelColor(x, y, QColor(100, 100, 100, image.pixelColor(x, y).alpha()));
                return QIcon(QPixmap::fromImage(image));
            }
        }
        case Qt::UserRole:
            return m_data[row].apertureId;
        default:
            break;
        }
    } else {
        if (m_data[row].toolId == -1)
            switch (role) {
            case Qt::DisplayRole:
                return tr("Select Tool");
            case Qt::TextAlignmentRole:
                return Qt::AlignCenter;
            case Qt::UserRole:
                return m_data[row].toolId;
            default:
                break;
            }
        else
            switch (role) {
            case Qt::DisplayRole:
                return App::toolHolder().tool(m_data[row].toolId).name();
            case Qt::DecorationRole:
                return App::toolHolder().tool(m_data[row].toolId).icon();
            case Qt::UserRole:
                return m_data[row].toolId;
            default:
                break;
            }
    }
    return QVariant();
}

QVariant DrillModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case Name:
                return m_type == tAperture ? tr("Aperture") : tr("Tool");
            case Tool:
                return tr("Tool");
            }
        }
        return QString(m_type == tAperture ? "D%1" : "T%1").arg(m_data[section].apertureId);
    case Qt::SizeHintRole:
        if (orientation == Qt::Vertical)
            return QFontMetrics(QFont()).boundingRect(QString("T999")).size() + QSize(Header::DelegateSize + 10, 1);
        return QVariant();
    case Qt::TextAlignmentRole:
        if (orientation == Qt::Vertical)
            return static_cast<int>(Qt::AlignRight) | static_cast<int>(Qt::AlignVCenter);
        return Qt::AlignCenter;
    default:
        return QVariant();
    }
}

Qt::ItemFlags DrillModel::flags(const QModelIndex& index) const {
    if (index.column() == Name)
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
