#include "gerbernode.h"
#include "project.h"
#include <QFileInfo>
#include <mainwindow.h>

QTimer GerberNode::m_repaintTimer;

GerberNode::GerberNode(int id)
    : AbstractNode(id)
{
    Project::file<Gerber::File>(m_id)->addToScene();
    connect(&m_repaintTimer, &QTimer::timeout, this, &GerberNode::repaint);
    m_repaintTimer.setSingleShot(true);
    m_repaintTimer.start(100);
}

GerberNode::~GerberNode()
{
    m_repaintTimer.start(10);
}

bool GerberNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case Name:
        switch (role) {
        case Qt::CheckStateRole:
            Project::file(m_id)->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
    case Layer:
        switch (role) {
        case Qt::EditRole:
            Project::file(m_id)->setSide(static_cast<Side>(value.toBool()));
            return true;
        default:
            return false;
        }
    case Other:
        switch (role) {
        case Qt::CheckStateRole:
            m_current = value.value<Qt::CheckState>();
            return true;
        default:
            return false;
        }
    default:
        return false;
    }
}

Qt::ItemFlags GerberNode::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch (index.column()) {
    case Name:
        return itemFlag | Qt::ItemIsUserCheckable;
    case Layer:
        return itemFlag | Qt::ItemIsEditable;
    case Other:
        return itemFlag | Qt::ItemIsUserCheckable;
    default:
        return itemFlag;
    }
}

QVariant GerberNode::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case Name:
        switch (role) {
        case Qt::DisplayRole:
            return Project::file(m_id)->shortName();
        case Qt::ToolTipRole:
            return Project::file(m_id)->shortName() + "\n"
                + Project::file(m_id)->name();
        case Qt::CheckStateRole:
            return Project::file(m_id)->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole: {
            QColor color(Project::file(m_id)->color());
            color.setAlpha(255);
            QPixmap pixmap(22, 22);
            pixmap.fill(Qt::transparent);
            QPainter p(&pixmap);
            p.setBrush(color);
            p.drawRect(3, 3, 15, 15);
            if (Project::file<Gerber::File>(m_id)->itemsType() == Gerber::File::Raw) {
                QFont f;
                f.setBold(true);
                p.setFont(f);
                p.drawText(QRect(0, 0, 22, 20), Qt::AlignCenter, "R");
            }
            return pixmap;
        }
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case Layer:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return tr("Top|Bottom").split('|')[Project::file(m_id)->side()];
        case Qt::EditRole:
            return static_cast<bool>(Project::file(m_id)->side());
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case Other:
        switch (role) {
        case Qt::CheckStateRole:
            return m_current;
        default:
            return QVariant();
        }
    default:
        break;
    }
    return QVariant();
}

QTimer* GerberNode::repaintTimer()
{
    return &m_repaintTimer;
}

void GerberNode::repaint()
{
    const int count = m_parentItem->childCount();
    const int k = static_cast<int>((count > 1) ? (200.0 / (count - 1)) * row() : 0);
    //    int k = -60 + static_cast<int>((count > 1) ? (240.0 / (count - 1)) * row() : 0);
    //    if (k < 0)
    //        k += 360;
    Project::file<Gerber::File>(m_id)->setColor(QColor::fromHsv(k, /*255 - k * 0.2*/ 255, 255, 150));
    Scene::update();
}
