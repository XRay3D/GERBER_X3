#ifndef ICONS_H
#define ICONS_H

#include <QIcon>

enum Id {
    GCodePropertiesIcon,
    AutoRefpointsIcon,

    FolderIcon,
    ToolFolderIcon,

    HideOtherIcon,

    SettingsIcon,

    ToolDatabaseIcon,

    Zoom100Icon,
    ZoomFitIcon,
    ZoomInIcon,
    ZoomOutIcon,
    ZoomToSelectedIcon,
    SelectAllIcon,

    ButtonAddBridgeIcon,
    ButtonCloseIcon,
    ButtonCreateIcon,
    ButtonEditIcon,
    ButtonSelectIcon,

    OpenFileIcon,
    RemoveIcon,
    DeleteIcon,

    CopyIcon_,
    NewToolIcon,
    NewGroupIcon,

    CloseAllIcon,
    CloseIcon,
    PrintIcon,

    SaveAllIcon,
    SaveIcon,
    SavePdfIcon,
    ExitIcon,

    SaveAsIcon,
    OpenProjectIcon,
    NewProjectIcon,

};

static QIcon Icon(int id)
{
    switch (id) {
    case GCodePropertiesIcon:
        return QIcon::fromTheme("node");
    case AutoRefpointsIcon:
        return QIcon::fromTheme("snap-nodes-cusp");

    case FolderIcon:
        return QIcon::fromTheme("folder");
    case ToolFolderIcon:
        return QIcon::fromTheme("folder-sync");

    case HideOtherIcon:
        return QIcon::fromTheme("hint");

    case SettingsIcon:
        return QIcon::fromTheme("configure-shortcuts");

    case ToolDatabaseIcon:
        return QIcon::fromTheme("view-form");

    case Zoom100Icon:
        return QIcon::fromTheme("zoom-original");
    case ZoomFitIcon:
        return QIcon::fromTheme("zoom-fit-best");
    case ZoomInIcon:
        return QIcon::fromTheme("zoom-in");
    case ZoomOutIcon:
        return QIcon::fromTheme("zoom-out");
    case ZoomToSelectedIcon:
        return QIcon::fromTheme("zoom-to-selected");
    case SelectAllIcon:
        return QIcon::fromTheme("edit-select-all");

    case ButtonAddBridgeIcon:
        return QIcon::fromTheme("edit-cut");
    case ButtonCloseIcon:
        return QIcon::fromTheme("window-close");
    case ButtonCreateIcon:
        return QIcon::fromTheme("document-export");
    case ButtonEditIcon:
        return QIcon::fromTheme("document-edit");
    case ButtonSelectIcon:
        return QIcon::fromTheme("view-form");
        //return QIcon::fromTheme("tools-wizard");

    case OpenFileIcon:
        return QIcon::fromTheme("document-open");
    case RemoveIcon:
        return QIcon::fromTheme("list-remove");
    case DeleteIcon:
        return QIcon::fromTheme("edit-delete");
    case CopyIcon_:
        return QIcon::fromTheme("edit-copy");
    case NewToolIcon:
        return QIcon::fromTheme("list-add");
    case NewGroupIcon:
        return QIcon::fromTheme("folder-add");
    case CloseAllIcon:
        return QIcon::fromTheme("list-remove");
    case CloseIcon:
        return QIcon::fromTheme("document-close");
    case PrintIcon:
        return QIcon::fromTheme("document-print");
    case SaveAllIcon:
        return QIcon::fromTheme("document-save-all");
    case SaveIcon:
        return QIcon::fromTheme("document-save");
    case SavePdfIcon:
        return QIcon::fromTheme("acrobat");
    case ExitIcon:
        return QIcon::fromTheme("application-exit");

    case SaveAsIcon:
        return QIcon::fromTheme("document-save-as");
    case OpenProjectIcon:
        return QIcon::fromTheme("project-open");
    case NewProjectIcon:
        return QIcon::fromTheme("project-development-new-template");
    }
    return QIcon();
}

#endif // ICONS_H
