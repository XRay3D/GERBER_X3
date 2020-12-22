#include "headerparser.h"
#include "dxffile.h"
#include <QTreeWidget>

namespace Dxf {
SectionHEADER::SectionHEADER(File* file, QVector<CodeData>&& data)
    : SectionParser(std::move(data),file)
    , header(file->header())
{
}

void SectionHEADER::parse()
{
    CodeData code;
    QString key;
    while (code != "ENDSEC") {
        // Прочитать другую пару код / значение
        code = nextCode();
        if (code.type() == CodeData::String && QString(code).startsWith('$')) {
            key = code.code();
        } else if (code == "ENDSEC") {
            break;
        } else if (!key.isEmpty()) {
            header.data[key][code.code()] = code.value();
        }
    }
    if constexpr (0) {
        auto tw = new QTreeWidget();
        tw->setWindowTitle("Section HEADER");
        tw->setColumnCount(2);
        auto iPar = header.data.constBegin();
        QList<QTreeWidgetItem*> items;
        while (iPar != header.data.constEnd()) {
            items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(iPar.key())));
            {
                auto iVal = iPar.value().constBegin();
                while (iVal != iPar.value().constEnd()) {
                    items.last()->addChild(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr), { QString::number(iVal.key()), iVal.value().toString() }));
                    ++iVal;
                }
            }
            ++iPar;
        }
        tw->insertTopLevelItems(0, items);
        tw->setWindowFlag(Qt::WindowStaysOnTopHint, true);
        tw->expandAll();
        tw->show();
    }
}
}
