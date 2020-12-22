#include "dxffile.h"

#include "dxfvalues.h"

#include "section/blocks.h"
#include "section/classes.h"
#include "section/entities.h"
#include "section/headerparser.h"
#include "section/objects.h"
#include "section/sectionparser.h"
#include "section/tables.h"
#include "section/thumbnailimage.h"

#include "tables/layer.h"

#include "gi/aperturepathitem.h"
#include "gi/pathitem.h"

#include <QDebug>

namespace Dxf {

File* File::self;

File::File()
{
    self = this;
}

File::~File() { qDeleteAll(sections); }

LAYER* File::layer(const QString& name)
{
    if (self->m_layers.find(name) != self->m_layers.end())
        return self->m_layers[name];
    return nullptr;
}
}

ItemGroup* Dxf::File::itemGroup() const
{
    return m_itemGroup.first();
}

FileType Dxf::File::type() const { return FileType::Dxf; }

void Dxf::File::createGi()
{
    //    int i = 0;
    for (auto& [name, layer] : m_layers) {
        //        if (i++)
        //            m_itemGroup.append(new ItemGroup);
        for (auto& go : layer->gig) {
            //            auto gi = new AperturePathItem(go.path(), this);
            auto gi = new PathItem(go.path());
            const QColor& c(dxfColors[layer->colorNumber]);
            gi->setPenColor(&c);
            m_itemGroup.last()->append(gi);
        }
        //        m_itemGroup.last()->setVisible(true);
    }
    m_itemGroup.last()->setVisible(true);
}

void Dxf::File::write(QDataStream& stream) const
{
    //    stream << *static_cast<const QList<GraphicObject>*>(this); // write  QList<GraphicObject>
    //    stream << m_apertures;
    //    stream << m_format;
    //    //stream << layer;
    //    //stream << miror;
    //    stream << rawIndex;
    //    stream << m_itemsType;
    //    stream << m_components;
    //    //    stream << *static_cast<const AbstractFile*>(this);
    //    //_write(stream);
}

void Dxf::File::read(QDataStream& stream)
{
    //    crutch = &m_format; ///////////////////
    //    stream >> *static_cast<QList<GraphicObject>*>(this); // read  QList<GraphicObject>
    //    stream >> m_apertures;
    //    stream >> m_format;
    //    //stream >> layer;
    //    //stream >> miror;
    //    stream >> rawIndex;
    //    stream >> m_itemsType;
    //    stream >> m_components;
    //    for (GraphicObject& go : *this) {
    //        go.m_gFile = this;
    //        go.m_state.m_format = format();
    //    }
    //    //    stream >> *static_cast<AbstractFile*>(this);
    //    //_read(stream);
}

Paths Dxf::File::merge() const
{
    return {};
}
