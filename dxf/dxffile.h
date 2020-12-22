#pragma once

// https://help.autodesk.com/view/OARX/2020/RUS/?guid=GUID-235B22E0-A567-4CF6-92D3-38A2306D73F3
#include "abstractfile.h"
#include "block.h"
#include "codedata.h"
#include "dxfvalues.h"
#include "header.h"
#include <QDebug>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QVector>
#include <algorithm>
#include <forward_list>
#include <gi/itemgroup.h>

namespace Dxf {

struct SectionParser;

struct LAYER;

using Layers = std::map<QString, LAYER*>;

class File : public AbstractFile {
    friend class Parser;

public:
    explicit File();
    ~File();
    static LAYER* layer(const QString& name);
    Header& header() { return m_header; }
    Layers& layers() { return m_layers; }
    QMap<QString, Block*>& blocks() { return m_blocks; }

private:
    int ctr = 0;
    static File* self;
    QVector<SectionParser*> sections;
    QMap<QString, Block*> m_blocks;
    Header m_header;
    Layers m_layers;

    // AbstractFile interface
public:
    ItemGroup* itemGroup() const override;
    FileType type() const override;
    void createGi() override;

protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    Paths merge() const override;
};

}
