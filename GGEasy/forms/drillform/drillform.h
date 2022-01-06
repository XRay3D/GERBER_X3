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
#pragma once
#ifdef GBR_
#include "extypes.h"
#include "gbrtypes.h"
#endif

#include "gcode/gcode.h"

#include <QHeaderView>
#include <QWidget>

namespace Ui {
class DrillForm;
}

namespace Gerber {
class AbstractAperture;
}

class DrillModel;
class AbstractDrillPrGI;
class Header;
class QCheckBox;

class DrillForm : public QWidget {
    Q_OBJECT

public:
    explicit DrillForm(QWidget* parent = nullptr);
    ~DrillForm() override;

#ifdef GBR_
    void setExcellonTools(const Excellon::Tools& value);
#endif
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

    void updateToolsOnGi(int apToolId);
    void pickUpTool();

    //    inline void updateCreateButton();
    inline void setSelected(int id, bool fl);
    inline void zoonToSelected();
    inline void deselectAll();
    DrillModel* model = nullptr;
    Ui::DrillForm* ui;

    int m_type;
#ifdef GBR_
    Gerber::ApertureMap m_apertures;
    Excellon::Tools m_tools;
#endif
    DrillPreviewGiMap m_giPeview;
    FileInterface* file = nullptr;
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
    static QRect getRect(const QRect& rect);

signals:
    void onChecked(int = -1);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

private:
    int flag = Qt::Unchecked;
    mutable mvector<QRect> m_checkRect;
    void setChecked(int index, bool ch);
    bool checked(int index) const;
    DrillModel* model() const;
};

#include "app.h"
