/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "gbr_aperture.h"
#include "gbr_types.h"
#include "gbrcomp_onent.h"

#include "fileifce.h"
#include "gi_group.h"

#include <QDebug>
#include <forward_list>

namespace Gerber {

class File : public FileInterface {
    friend class Parser;
    friend class Plugin;
    friend QDataStream& operator>>(QDataStream& stream, std::shared_ptr<AbstractAperture>& aperture);

public:
    explicit File(const QString& name = "");
    ~File() override;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    const Format& format() const { return format_; }
    Format& format() { return format_; }
    Pathss& groupedPaths(Group group = CopperGroup, bool fl = false);
    bool flashedApertures() const;
    const ApertureMap* apertures() const;

    enum ItemsType {
        NullType = -1,
        Normal,
        ApPaths,
        Components,
    };
    // FileInterface interface
    void setItemType(int type) override;
    int itemsType() const override;
    void initFrom(FileInterface* file) override;
    FileTree_::Node* node() override;

    FileType type() const override { return FileType::Gerber_; }

    void setColor(const QColor& color) override;

    mvector<const AbstrGraphicObject*> graphicObjects() const override;
    const auto& graphicObjects2() const { return graphicObjects_; };

protected:
    Paths merge() const override;

private:
    QList<Comp::Component> components_;
    mvector<GraphicObject> graphicObjects_;
    ApertureMap apertures_;
    void grouping(PolyTree& node, Pathss* pathss);
    Format format_;
    Group group_ {};
    // Layer layer = Copper;
    // Miror miror = Vertical;
    // QPointf offset;

    QVector<int> rawIndex;
    std::forward_list<Path> checkList;
    static inline File* crutch;

    // FileTree_::Node interface
protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

    // FileTree_::Node interface
public:
    void createGi() override;
    const QList<Comp::Component>& components() const;
};

} // namespace Gerber

Q_DECLARE_METATYPE(Gerber::File)
