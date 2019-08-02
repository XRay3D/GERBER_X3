#ifndef DRILLFORM_H
#define DRILLFORM_H

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
class PreviewItem;
class QCheckBox;
class QGridLayout;

class DrillForm : public QWidget {
    Q_OBJECT

public:
    explicit DrillForm(QWidget* parent = nullptr);
    ~DrillForm() override;
    static DrillForm* self;

    void setApertures(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value);
    void setHoles(const QMap<int, double>& value);
    void updateFiles();

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

    inline void updateCreateButton();
    inline void setSelected(int id, bool fl);
    inline void zoonToSelected();
    inline void deselectAll();
    DrillModel* model;
    Ui::DrillForm* ui;

    int m_type;
    QMap<int, QSharedPointer<Gerber::AbstractAperture>> m_apertures;
    QMap<int, double> m_tools;
    QMap<int, QVector<QSharedPointer<PreviewItem>>> m_sourcePreview;

    QCheckBox* cbx;
    QGridLayout* lay;
    AbstractFile* file = nullptr;

    void clear();
};

#endif // DRILLFORM_H
