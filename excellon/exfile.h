#pragma once

#include "extypes.h"
#include "abstractfile.h"

namespace Excellon {

class File : public AbstractFile, public QList<Hole> {
    Tools m_tools;
    friend class Parser;
    Format m_format;

public:
    explicit File();
    ~File() override;

    FileType type() const override { return FileType::Excellon; }

    double tool(int t) const;
    Tools tools() const;

    Format format() const;
    void setFormat(const Format& value);

    ItemGroup* itemGroup() const override { return m_itemGroup.last(); }

protected:
    Paths merge() const override;

    // AbstractFile interface
public:
    void createGi() override;

protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
};
} // namespace Excellon
