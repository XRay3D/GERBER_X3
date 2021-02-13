#include "style.h"

#include <QPainter>
#include <QStyleOption>

Style::Style() { }

Style::~Style() { }

void Style::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    switch (element) {
    case PE_Frame:
        break;
    case PE_FrameDefaultButton:
        break;
    case PE_FrameDockWidget:
        break;
    case PE_FrameFocusRect:
        break;
    case PE_FrameGroupBox:
        break;
    case PE_FrameLineEdit:
        break;
    case PE_FrameMenu:
        break;
    case PE_FrameTabWidget:
        break;
    case PE_FrameWindow:
        break;
    case PE_FrameButtonBevel:
        break;
    case PE_FrameButtonTool:
        break;
    case PE_FrameTabBarBase:
        break;
    case PE_PanelButtonCommand:
        break;
    case PE_PanelButtonBevel:
        break;
    case PE_PanelButtonTool:
        break;
    case PE_PanelMenuBar:
        break;
    case PE_PanelToolBar:
        break;
    case PE_PanelLineEdit:
        break;
    case PE_IndicatorArrowDown:
        break;
    case PE_IndicatorArrowLeft:
        break;
    case PE_IndicatorArrowRight:
        break;
    case PE_IndicatorArrowUp:
        break;
    case PE_IndicatorBranch:
        break;
    case PE_IndicatorButtonDropDown:
        break;
    case PE_IndicatorCheckBox:
        break;
    case PE_IndicatorDockWidgetResizeHandle:
        break;
    case PE_IndicatorHeaderArrow:
        break;
    case PE_IndicatorMenuCheckMark:
        break;
    case PE_IndicatorProgressChunk:
        break;
    case PE_IndicatorRadioButton:
        break;
    case PE_IndicatorSpinDown:
        break;
    case PE_IndicatorSpinMinus:
        break;
    case PE_IndicatorSpinPlus:
        break;
    case PE_IndicatorSpinUp:
        break;
    case PE_IndicatorToolBarHandle:
        break;
    case PE_IndicatorToolBarSeparator:
        break;
    case PE_PanelTipLabel:
        break;
    case PE_IndicatorTabTear: //PE_IndicatorTabTearLeft
        break;
    case PE_PanelScrollAreaCorner:
        break;
    case PE_Widget:
        break;
    case PE_IndicatorColumnViewArrow:
        break;
    case PE_IndicatorItemViewItemDrop:
        break;
    case PE_PanelItemViewItem:
        break;
    case PE_PanelStatusBar:
        break;
    case PE_IndicatorTabClose:
        break;
    case PE_PanelMenu:
        break;
    case PE_IndicatorTabTearRight:
        break;
        // do not add any values below/greater this
    case PE_CustomBase:
        break;
    }
    return;
    if (element == PE_IndicatorSpinUp || element == PE_IndicatorSpinDown) {
        QPolygon points(3);
        int x = option->rect.x();
        int y = option->rect.y();
        int w = option->rect.width() / 2;
        int h = option->rect.height() / 2;
        x += (option->rect.width() - w) / 2;
        y += (option->rect.height() - h) / 2;

        if (element == PE_IndicatorSpinUp) {
            points[0] = QPoint(x, y + h);
            points[1] = QPoint(x + w, y + h);
            points[2] = QPoint(x + w / 2, y);
        } else { // PE_SpinBoxDown
            points[0] = QPoint(x, y);
            points[1] = QPoint(x + w, y);
            points[2] = QPoint(x + w / 2, y + h);
        }

        //        if (option->state & State_Enabled) {
        //            painter->setPen(option->palette.mid().color());
        //            painter->setBrush(option->palette.buttonText());
        //        } else {
        //            painter->setPen(option->palette.buttonText().color());
        //            painter->setBrush(option->palette.mid());
        //        }
        //        painter->drawPolygon(points);
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
void Style::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    auto Button = static_cast<const QStyleOptionButton*>(option);
    auto Complex = static_cast<const QStyleOptionComplex*>(option);
    auto DockWidget = static_cast<const QStyleOptionDockWidget*>(option);
    auto FocusRect = static_cast<const QStyleOptionFocusRect*>(option);
    auto Frame = static_cast<const QStyleOptionFrame*>(option);
    auto GraphicsItem = static_cast<const QStyleOptionGraphicsItem*>(option);
    auto Header = static_cast<const QStyleOptionHeader*>(option);
    auto MenuItem = static_cast<const QStyleOptionMenuItem*>(option);
    auto ProgressBar = static_cast<const QStyleOptionProgressBar*>(option);
    auto RubberBand = static_cast<const QStyleOptionRubberBand*>(option);
    auto Tab = static_cast<const QStyleOptionTab*>(option);
    auto TabBarBase = static_cast<const QStyleOptionTabBarBase*>(option);
    auto TabWidgetFrame = static_cast<const QStyleOptionTabWidgetFrame*>(option);
    auto ToolBar = static_cast<const QStyleOptionToolBar*>(option);
    auto ToolBox = static_cast<const QStyleOptionToolBox*>(option);
    auto ViewItem = static_cast<const QStyleOptionViewItem*>(option);

    switch (element) {
    case CE_PushButton:
    case CE_PushButtonBevel:
    case CE_PushButtonLabel:
        painter->drawText(option->rect, Qt::AlignCenter, Button->text);
        painter->drawRect(option->rect);
        return;

    case CE_CheckBox:
    case CE_CheckBoxLabel:
        painter->drawText(option->rect, Qt::AlignCenter, Button->text);
        painter->drawRect(option->rect);
        return;

    case CE_RadioButton:
    case CE_RadioButtonLabel:
        painter->drawText(option->rect, Qt::AlignCenter, Button->text);
        painter->drawRect(option->rect);
        return;

    case CE_TabBarTab:
    case CE_TabBarTabShape:
    case CE_TabBarTabLabel:
        painter->drawText(option->rect, Qt::AlignCenter, Tab->text);
        painter->drawRect(option->rect);
        return;

    case CE_ProgressBar:
    case CE_ProgressBarGroove:
    case CE_ProgressBarContents:
    case CE_ProgressBarLabel:
        painter->drawText(option->rect, Qt::AlignCenter, ProgressBar->text);
        painter->drawRect(option->rect);
        return;

    case CE_MenuItem:
    case CE_MenuScroller:
    case CE_MenuVMargin:
    case CE_MenuHMargin:
    case CE_MenuTearoff:
    case CE_MenuEmptyArea:

    case CE_MenuBarItem:
    case CE_MenuBarEmptyArea:
        painter->drawText(option->rect, Qt::AlignCenter, MenuItem->text);
        painter->drawRect(option->rect);
        return;

    case CE_ToolButtonLabel:
        painter->drawText(option->rect, Qt::AlignCenter, Button->text);
        painter->drawRect(option->rect);
        return;

    case CE_Header:
        return;
    case CE_HeaderSection:
        return;
    case CE_HeaderLabel:
        return;

    case CE_ToolBoxTab:
        return;
    case CE_SizeGrip:
        return;
    case CE_Splitter:
        return;
    case CE_RubberBand:
        painter->setPen(Qt::red);
        painter->drawRect(option->rect + QMargins(-1, +0, +0, -1));
        return;
    case CE_DockWidgetTitle:
        return;

    case CE_ScrollBarAddLine:
        return;
    case CE_ScrollBarSubLine:
        return;
    case CE_ScrollBarAddPage:
        return;
    case CE_ScrollBarSubPage:
        return;
    case CE_ScrollBarSlider:
        return;
    case CE_ScrollBarFirst:
        return;
    case CE_ScrollBarLast:
        return;

    case CE_FocusFrame:
        return;
    case CE_ComboBoxLabel:
        return;

    case CE_ToolBar:
        return;
    case CE_ToolBoxTabShape:
        return;
    case CE_ToolBoxTabLabel:
        return;
    case CE_HeaderEmptyArea:
        return;

    case CE_ColumnViewGrip:
        return;

    case CE_ItemViewItem:
        return;

    case CE_ShapedFrame:
        return;
    }
}
void Style::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const
{

    auto ComboBox = static_cast<const QStyleOptionComboBox*>(option);
    auto GroupBox = static_cast<const QStyleOptionGroupBox*>(option);
    auto SizeGrip = static_cast<const QStyleOptionSizeGrip*>(option);
    auto Slider = static_cast<const QStyleOptionSlider*>(option);
    auto SpinBox = static_cast<const QStyleOptionSpinBox*>(option);
    auto TitleBar = static_cast<const QStyleOptionTitleBar*>(option);
    auto ToolButton = static_cast<const QStyleOptionToolButton*>(option);

    painter->drawRect(option->rect);
    return;
    switch (control) {
    case CC_SpinBox:
        return;
    case CC_ComboBox:
        return;
    case CC_ScrollBar:
        return;
    case CC_Slider:
        return;
    case CC_ToolButton:
        return;
    case CC_TitleBar:
        return;
    case CC_Dial:
        return;
    case CC_GroupBox:
        return;
    case CC_MdiControls:
        return;
    }
}
void Style::drawItemText(QPainter* painter, const QRect& rect, int flags, const QPalette& pal, bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
    //painter->drawText(rect, text);
    //    painter->setPen(Qt::black);
    //    painter->drawText(rect, text);
}
void Style::drawItemPixmap(QPainter* painter, const QRect& rect, int alignment, const QPixmap& pixmap) const
{
    painter->drawPixmap(rect, pixmap, pixmap.rect());
}
