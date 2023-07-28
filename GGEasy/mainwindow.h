/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
// #include "a_pch.h"

#include "app.h"
#include "recent.h"
#include "settingsdialog.h"

#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QThread>
#include <QUndoStack>

namespace FileTree {
class View;
}

namespace GCode {
class File;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    //    friend void FileTree::View::on_doubleClicked(const QModelIndex&);
    friend class Recent;
    friend class Project;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void init();

    // QMainWindow interface
    QMenu* createPopupMenu() override;
    const QDockWidget* dockWidget() const;
    QDockWidget* dockWidget();

    static void translate(const QString& locale) {
        static std::vector<std::unique_ptr<QTranslator>> translators;
        translators.clear();
        QDir dir(qApp->applicationDirPath() + "/translations");
        for (auto&& str: dir.entryList(QStringList{"*" + locale + ".qm"}, QDir::Files)) {
            translators.emplace_back(std::make_unique<QTranslator>());
            if (translators.back()->load(str, dir.path()))
                qApp->installTranslator(translators.back().get());
        }
    }
    void loadFile(const QString& fileName);
    static void updateTheme() {
        if (App::settings().theme()) {

            static const char* const dwCloseXpm[] = {
                "11 13 3 1",
                "  c None",
                "@ c #6C6A67",
                "$ c #6C6A67", // B5B0AC
                "           ",
                "           ",
                "           ",
                "  $@   @$  ",
                "  @@@ @@@  ",
                "   @@@@@   ",
                "    @@@    ",
                "   @@@@@   ",
                "  @@@ @@@  ",
                "  $@   @$  ",
                "           ",
                "           ",
                "           ",
            };

            static const char* const dwRestoreXpm[] = {
                "11 13 3 1",
                "  c None",
                "@ c #6C6A67",
                "# c #6C6A67", // ABA6A3
                "           ",
                "           ",
                "           ",
                "    #@@@#  ",
                "    @   @  ",
                "  #@@@# @  ",
                "  @   @ @  ",
                "  @   @@@  ",
                "  @   @    ",
                "  #@@@#    ",
                "           ",
                "           ",
                "           ",
            };

            static const char* const dwMinimizeXpm[] = {
                "11 13 2 1",
                "  c None",
                "@ c #6C6A67",
                "           ",
                "           ",
                "           ",
                "           ",
                "           ",
                "           ",
                "  @@@@@@@  ",
                "  @@@@@@@  ",
                "           ",
                "           ",
                "           ",
                "           ",
                "           ",
            };

            static const char* const qtTitlebarContextHelp[] = {
                "10 10 3 1",
                "  c None",
                "# c #000000",
                "+ c #444444",
                "  +####+  ",
                " ###  ### ",
                " ##    ## ",
                "     +##+ ",
                "    +##   ",
                "    ##    ",
                "    ##    ",
                "          ",
                "    ##    ",
                "    ##    ",
            };

            class Style : public QProxyStyle {
            public:
                Style()
                    : QProxyStyle("Fusion") { }
                QPixmap getPixmap(StandardPixmap standardPixmap) const {
                    switch (standardPixmap) {
                    case SP_TitleBarNormalButton:
                        return QPixmap(dwRestoreXpm);
                    case SP_TitleBarMinButton:
                        return QPixmap(dwMinimizeXpm);
                    case SP_TitleBarCloseButton:
                    case SP_DockWidgetCloseButton:
                        return QPixmap(dwCloseXpm);
                    default:
                        return {};
                    }
                }

                QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption* option, const QWidget* widget) const override {
                    if (auto pix = getPixmap(standardIcon); !pix.isNull())
                        return pix;
                    return QProxyStyle::standardIcon(standardIcon, option, widget);
                }
                //            QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption* opt, const QWidget* widget) const override {
                //                if(auto pix = getPixmap(standardPixmap); !pix.isNull())
                //                    return pix;
                //                return QProxyStyle::standardPixmap(standardPixmap, opt, widget);
                //            }
                void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override {
                    if (element == QStyle::PE_IndicatorBranch) {
                        auto r = option->rect;
                        auto c = r.center();
                        auto color = qApp->palette().color(QPalette::Highlight);
                        painter->setPen(color);
                        if (!(option->state & State_Children)) {
                            if (option->state & State_Sibling) // The node in the tree has a sibling (i.e., there is another node in the same column).
                                painter->drawLine(c.x(), r.top(), c.x(), r.bottom());
                            if (option->state & State_Item) { // This branch indicator has an item.
                                painter->drawLine(c.x(), r.top(), c.x(), c.y());
                                painter->drawLine(c.x(), c.y(), r.right(), c.y());
                            }
                        }
                        //                    if(option->state & State_Children) // The branch has children (i.e., a new sub-tree can be opened at the branch).
                        //                        painter->fillRect(option->rect, Qt::blue);
                        //                    if(option->state & State_Open) // The branch indicator has an opened sub-tree.
                        //                        painter->fillRect(option->rect, Qt::yellow);
                    }
                    QProxyStyle::drawPrimitive(element, option, painter, widget);
                }

                //            void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override {
                //                if(element == QStyle::PE_IndicatorBranch) {
                //                    auto r = option->rect;
                //                    auto c = r.center();
                //                    auto color = Qt::darkGray; // qApp->palette().color(QPalette::Highlight);
                //                    painter->setPen(color);
                //                    if(!(option->state & State_Children)) {
                //                        if(option->state & State_Sibling) // The node in the tree has a sibling (i.e., there is another node in the same column).
                //                            painter->drawLine(c.x(), r.top(), c.x(), r.bottom());
                //                        if(option->state & State_Item) {  // This branch indicator has an item.
                //                            painter->drawLine(c.x(), r.top(), c.x(), c.y());
                //                            painter->drawLine(c.x(), c.y(), r.right(), c.y());
                //                        }
                //                    }
                //                    //                    if(option->state & State_Children) // The branch has children (i.e., a new sub-tree can be opened at the branch).
                //                    //                        painter->fillRect(option->rect, Qt::blue);
                //                    //                    if(option->state & State_Open) // The branch indicator has an opened sub-tree.
                //                    //                        painter->fillRect(option->rect, Qt::yellow);
                //                }
                //                QProxyStyle::drawPrimitive(element, option, painter, widget);
                //            }
            };

            qApp->setStyle(new Style);

            QColor baseColor;
            QColor disabledColor;
            QColor highlightColor;
            QColor linkColor;
            QColor windowColor;
            QColor windowTextColor;

            switch (App::settings().theme()) {
            case LightBlue:
                baseColor = QColor(230, 230, 230);
                disabledColor = QColor(127, 127, 127);
                highlightColor = QColor(61, 174, 233);
                linkColor = QColor(61, 174, 233);
                windowColor = QColor(200, 200, 200);
                windowTextColor = QColor(0, 0, 0);
                break;
            case LightRed:
                baseColor = QColor(230, 230, 230);
                disabledColor = QColor(127, 127, 127);
                highlightColor = QColor(218, 68, 83);
                linkColor = QColor(61, 174, 233);
                windowColor = QColor(200, 200, 200);
                windowTextColor = QColor(0, 0, 0);
                break;
            case DarkBlue:
                baseColor = QColor(20, 20, 20);
                disabledColor = QColor(80, 80, 80);
                highlightColor = QColor(61, 174, 233);
                linkColor = QColor(61, 174, 233);
                windowColor = QColor(30, 30, 30);
                windowTextColor = QColor(220, 220, 220);
                break;
            case DarkRed:
                baseColor = QColor(20, 20, 20);
                disabledColor = QColor(80, 80, 80);
                highlightColor = QColor(218, 68, 83);
                linkColor = QColor(61, 174, 233);
                windowColor = QColor(30, 30, 30);
                windowTextColor = QColor(220, 220, 220);
                break;
            }

            QPalette palette;

            palette.setBrush(QPalette::Text, windowTextColor);
            palette.setBrush(QPalette::ToolTipText, windowTextColor);
            palette.setBrush(QPalette::WindowText, windowTextColor);
            palette.setBrush(QPalette::ButtonText, windowTextColor);
            palette.setBrush(QPalette::HighlightedText, Qt::black);
            palette.setBrush(QPalette::BrightText, Qt::red);

            palette.setBrush(QPalette::Link, linkColor);
            palette.setBrush(QPalette::LinkVisited, highlightColor);

            palette.setBrush(QPalette::AlternateBase, windowColor);
            palette.setBrush(QPalette::Base, baseColor);
            palette.setBrush(QPalette::Button, windowColor);

            palette.setBrush(QPalette::Highlight, highlightColor);

            palette.setBrush(QPalette::ToolTipBase, windowTextColor);
            palette.setBrush(QPalette::Window, windowColor);

            palette.setBrush(QPalette::Disabled, QPalette::ButtonText, disabledColor);
            palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
            palette.setBrush(QPalette::Disabled, QPalette::Text, disabledColor);
            palette.setBrush(QPalette::Disabled, QPalette::Shadow, disabledColor);

            //        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, disabledColor);
            //        palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, disabledColor);
            //        palette.setBrush(QPalette::Inactive, QPalette::Text, disabledColor);
            //        palette.setBrush(QPalette::Inactive, QPalette::Shadow, disabledColor);

            qApp->setPalette(palette);
        } else {
#if __has_include("xrstyle.h") && 0
            QApplication::setStyle(new XrStyle);
#else
            qApp->setStyle(QStyleFactory::create("Fusion"));
            qApp->setPalette(QApplication::style()->standardPalette());
#endif
        }

        // if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows && QOperatingSystemVersion::current().majorVersion() > 7) {
        // App::mainWindow().setStyleSheet("QGroupBox, .QFrame {"
        // //"background-color: white;"
        // "border: 1px solid gray; }"
        // "QGroupBox { margin-top: 3ex; }" /* leave space at the top for the title */
        // "QGroupBox::title {"
        // "subcontrol-origin: margin;"
        // "subcontrol-position: top center; }" /* position at the top center */
        // );
        // } else {
        // App::mainWindow().setStyleSheet("QGroupBox, .QFrame {"
        // //"background-color: white;"
        // "border: 1px solid gray;"
        // "border-radius: 3px; }" // Win 7 or other
        // "QGroupBox { margin-top: 3ex; }" /* leave space at the top for the title */
        // "QGroupBox::title {"
        // "subcontrol-origin: margin;"
        // "subcontrol-position: top center; }" /* position at the top center */
        // );
        // }

        QIcon::setThemeName(App::settings().theme() < DarkBlue ? "ggeasy-light" : "ggeasy-dark");
        if (App::mainWindowPtr() && App::mainWindow().isVisible())
            SettingsDialog().show();
    }

    QUndoStack& undoStack() { return undoStack_; }

    void setDockWidget(QWidget* dwContent) {
        if (dwContent == nullptr || dockWidget_->widget() == dwContent)
            return;
        if (auto widget = dockWidget_->widget(); widget) {
            dockWidget_->setWidget(dwContent); // NOTE  заменяет виджет новым и сбрасывается предок
            widget->setParent(nullptr);        //       так как виджет лежит полем класса плагина.
        } else
            dockWidget_->setWidget(dwContent);
        dockWidget_->setWindowTitle(dwContent->windowTitle());
        if (auto pbClose{dwContent->findChild<QPushButton*>("pbClose")}; pbClose)
            connect(pbClose, &QPushButton::clicked, this, &MainWindow::resetToolPathsActions);
        dockWidget_->show();
    }

signals:
    void parseFile(const QString& filename, int type);

private slots:
    void fileError(const QString& fileName, const QString& error);
    void fileProgress(const QString& fileName, int max, int value);
    void addFileToPro(class AbstractFile* file);
    //    void setDockWidget(QWidget* dwContent);

private:
    QDockWidget* dockWidget_ = nullptr;
    Recent recentFiles;
    Recent recentProjects;

    QAction* closeAllAct_ = nullptr;
    QAction* redoAct = nullptr;
    QAction* undoAct = nullptr;

    QMenu* fileMenu = nullptr;
    QMenu* helpMenu = nullptr;
    QMenu* serviceMenu = nullptr;

    QString lastPath;
    QThread parserThread;

    QToolBar* fileToolBar = nullptr;
    QToolBar* toolpathToolBar = nullptr;
    QToolBar* zoomToolBar = nullptr;
    QUndoStack undoStack_;

    class Project* project_;
    bool openFlag;

    std::map<uint32_t, QAction*> toolpathActions;
    QActionGroup actionGroup;

    QMap<QString, class QProgressDialog*> progressDialogs_;
    QMessageBox reloadQuestion;

    void open();
    bool save();
    bool saveAs();

    void about();
    bool closeProject();

    void initWidgets();

    void printDialog();
    void renderPdf();

    void readSettings();

    void resetToolPathsActions() {
        if (auto widget = dockWidget_->widget(); widget) {
            dockWidget_->setWidget(new QWidget); // NOTE  заменяет виджет новым и сбрасывается предок
            widget->setParent(nullptr);          //       так как виджет лежит полем класса плагина.
        }
        dockWidget_->setVisible(false);
        if (auto action{actionGroup.checkedAction()}; action)
            action->setChecked(false);
    }

    void selectAll();
    void deSelectAll();

    void writeSettings();

    // create actions
    void createActions();
    void createActionsFile();
    void createActionsEdit();
    void createActionsService();
    void createActionsHelp();
    void createActionsZoom();
    void createActionsToolPath();
    void createActionsShape();

    void customContextMenuForToolBar(const QPoint& pos);

    // save GCode
    void saveGCodeFile(int32_t id);
    void saveGCodeFiles();
    void saveSelectedGCodeFiles();

    QString strippedName(const QString& fullFileName);

    void newFile();
    void documentWasModified();
    bool maybeSave();

    void editGcFile(GCode::File* file);

private:
    bool saveFile(const QString& fileName);
    void setCurrentFile(const QString& fileName);
    bool debug();

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    struct Ui {
        class QWidget* centralwidget;
        class QHBoxLayout* horizontalLayout;
        class GraphicsView* grView;
        class QMenuBar* menubar;
        class QStatusBar* statusbar;
        class QDockWidget* treeDockWidget;
        class QWidget* widget;
        class QVBoxLayout* verticalLayout;
        FileTree::View* treeView;
        void setupUi(QMainWindow* MainWindow);       // setupUi
        void retranslateUi(QMainWindow* MainWindow); // retranslateUi
    } ui;
};
