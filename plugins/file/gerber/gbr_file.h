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

#include "abstract_file.h"
#include "gbr_aperture.h"
#include "gbrcomp_onent.h"
#include <forward_list>

namespace Gerber {

class File : public AbstractFile {
    friend class Parser;
    friend class Plugin;
    friend QDataStream& operator>>(QDataStream& stream, std::shared_ptr<AbstractAperture>& aperture); // NOTE use private crutch

public:
    explicit File();
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
    // AbstractFile interface
    mvector<GraphicObject> getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test = {}) const override;
    void setItemType(int type) override;
    int itemsType() const override;
    void initFrom(AbstractFile* file) override;
    FileTree::Node* node() override;
    QIcon icon() const override;
    uint32_t type() const override { return GERBER; }
    void setColor(const QColor& color) override;

    mvector<const ::GraphicObject*> graphicObjects() const override;
    const auto& graphicObjects2() const { return graphicObjects_; };

protected:
    Paths merge() const override;

private:
    QList<Comp::Component> components_;
    mvector<GrObject> graphicObjects_;
    ApertureMap apertures_;
    void grouping(PolyTree& node, Pathss* pathss);
    Format format_;
    Group group_{};
    // Layer layer = Copper;

    QVector<int> rawIndex;
    std::forward_list<Path> checkList;
    static inline File* crutch;

    // FileTree::Node interface
protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

    // FileTree::Node interface
public:
    void createGi() override;
    const QList<Comp::Component>& components() const;
};

} // namespace Gerber

Q_DECLARE_METATYPE(Gerber::File)
