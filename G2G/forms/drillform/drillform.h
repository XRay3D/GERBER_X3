#ifndef DRILLFORM_H
#define DRILLFORM_H

#include <QHeaderView>
#include <QWidget>
#include <gccreator.h>
#include <gcfile.h>

namespace Ui {
class DrillForm;
}
namespace Gerber {
class AbstractAperture;
}

class DrillModel;
class DrillPrGI;
class Header;
class QCheckBox;

class DrillForm : public QWidget {
    Q_OBJECT

public:
    explicit DrillForm(QWidget* parent = nullptr);
    ~DrillForm() override;
    static DrillForm* self;

    void setApertures(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value);
    void setHoles(const QMap<int, double>& value);
    void updateFiles();
    static bool canToShow();

public slots:
    void on_pbClose_clicked();

private slots:
    void on_pbCreate_clicked();

private:
    GCode::GCodeType m_worckType = GCode::Drill;
    GCode::SideOfMilling m_side = GCode::Inner;

    void on_cbxFileCurrentIndexChanged(int index);
    void on_clicked(const QModelIndex& index);
    void on_doubleClicked(const QModelIndex& current);
    void on_currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_customContextMenuRequested(const QPoint& pos);

    void createHoles(int toolId, int toolIdSelected);
    void pickUpTool(int apertureId, double diameter, bool isSlot = false);

    //    inline void updateCreateButton();
    inline void setSelected(int id, bool fl);
    inline void zoonToSelected();
    inline void deselectAll();
    DrillModel* model = nullptr;
    Ui::DrillForm* ui;

    int m_type;
    QMap<int, QSharedPointer<Gerber::AbstractAperture>> m_apertures;
    QMap<int, double> m_tools;
    QMap<int, QVector<QSharedPointer<DrillPrGI>>> m_sourcePreview;
    AbstractFile* file = nullptr;
    QCheckBox* checkBox;
    Header* header;
    void clear();
};

class Header : public QHeaderView {
    Q_OBJECT

public:
    Header(Qt::Orientation orientation, QWidget* parent = nullptr);
    ~Header() override;

    enum {
        XOffset = 5,
        DelegateSize = 16
    };

    void setAll(bool ch);
    void togle(int index);
    void set(int index, bool ch);

    QVector<bool> checked();
    static QRect getRect(const QRect& rect);

signals:
    void onCheckedV(const QVector<bool>&);
    void onChecked(int);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

private:
    mutable QVector<bool> m_checked;
    mutable QVector<QRect> m_checkRect;
    void setChecked(int index, bool ch);
    bool checked(int index) const;
};

#endif // DRILLFORM_H
