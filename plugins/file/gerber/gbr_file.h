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

class[[= Serial::name("Gerber")]] File : public AbstractFile {
    friend class Parser;
    friend class Plugin;
    // NOTE use private crutch
    friend struct Serial::Adapter<std::shared_ptr<AbstractAperture>>; // NOTE use private crutch

public:
    explicit File();
    ~File() override;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    const Format& format() const { return format_; }
    Format& format() { return format_; }
    Geo::Polygons& groupedPaths(Group group = CopperGroup, bool fl = false);
    bool flashedApertures() const;
    const ApertureMap* apertures() const;

    enum ItemsType {
        NullType = -1,
        Normal,
        ApPaths,
        Components,
    };
    // AbstractFile interface
    std::vector<GraphicObject> getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test = {}) const override;
    void setItemType(int type) override;
    int32_t itemsType() const override;
    void initFrom(AbstractFile* file) override;
    FileTree::Node* node() override;
    QIcon icon() const override;
    uint32_t type() const override { return GERBER; }
    void setColor(const QColor& color) override;

    std::vector<const ::GraphicObject*> graphicObjects() const override;
    const auto& graphicObjects2() const { return graphicObjects_; };

protected:
    Geo::Polygons merge() const override;

private:
    // На каждый слитый полигон меди -- номера цепей объектов, что в нём оказались.
    std::vector<std::vector<int32_t>> copperNets(const Geo::Polygons& copper) const;
    QString netsToolTip(std::span<const int32_t> nets) const;

private:
    QList<Comp::Component> components_;
    QStringList nets_; // имена X2-цепей (%TO.N), индекс -- State::net()
    // Элементы сцены каждой цепи (обе группы, Normal и ApPaths), индекс -- номер
    // цепи. Собирается в createGi вместе с элементами, в проект не пишется.
    [[= Serial::skip]] std::vector<std::vector<Gi::Item*>> netItems_;
    std::vector<GrObject> graphicObjects_;
    ApertureMap apertures_;

    Format format_;
    [[= Serial::skip]] Group group_{};
    // Layer layer = Copper;

    QVector<int> rawIndex;
    [[= Serial::skip]] std::forward_list<Geo::Polyline> checkList; // рабочий список парсера
    static inline File* crutch;

    // FileTree::Node interface
protected:
public:
    void serialize(Serial::Writer& sb) const override { Serial::writeInto(sb, *this); }
    // Апертуры при чтении цепляются к текущему файлу — крюк ДО чтения полей.
    void preLoad() { crutch = this; }
    void postLoad() {
        for(GrObject& go: graphicObjects_) {
            go.gFile = this;
            go.state.file_ = this;
        }
        AbstractFile::postLoad();
    }

    // FileTree::Node interface
public:
    void createGi() override;
    const QList<Comp::Component>& components() const;
    QString netName(int32_t id) const { return nets_.value(id); }
    const QStringList& nets() const { return nets_; }
    std::span<Gi::Item* const> netItems(int32_t id) const {
        return id >= 0 && size_t(id) < netItems_.size() ? std::span{netItems_[id]} : std::span<Gi::Item* const>{};
    }
};

} // namespace Gerber

Q_DECLARE_METATYPE(Gerber::File)
