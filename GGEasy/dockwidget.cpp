//// This is an open source non-commercial project. Dear PVS-Studio, please check it.
//// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  03 October 2022                                                 *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2022                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// ********************************************************************************/
// #include "dockwidget.h"

// #include <QDialog>
// #include <QTimer>
// #include <qevent.h>

// DockWidget::DockWidget(QWidget* parent)
//     : QDockWidget(parent) {
//     setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
//     setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
//     hide();
//     setVisible(false);
// }

// void DockWidget::push(QWidget* w) {
//     if (widget())
//         widgets.push(widget());
//     if (w)
//         QDockWidget::setWidget(w);
// }

// void DockWidget::pop() {
//     if (widget()) {
//         if (widget()->objectName() == "ErrorDialog") {
//             static_cast<QDialog*>(widget())->reject();
//             QTimer::singleShot(1, [this] { widgets.pop(); });
//         } /* else if (widget()) {
//                  delete widget();
//              }*/
//     }
//     if (!widgets.isEmpty())
//         QDockWidget::setWidget(widgets.pop());
// }

//// void DockWidget::closeEvent(QCloseEvent* event) {
////     pop();
////     event->accept();
//// }

//// void DockWidget::showEvent(QShowEvent* event) {
////     event->ignore();
////     if (widget() == nullptr)
////         QTimer::singleShot(1, this, &QDockWidget::close);
//// }
