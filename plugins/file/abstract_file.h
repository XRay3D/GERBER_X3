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

#include "app.h"
#include "datastream.h"
#include "ft_node.h"
#include "gi_group.h"
#include "plugintypes.h"

#include "doublespinbox.h"

#include <QDateTime>
#include <QPainter>
#include <QSplashScreen>
#include <QtWidgets>
// #include <QApplication>
#include <QFileInfo>
// #include <QModelIndex>

inline QPixmap decoration(QColor color, QChar chr = {}) {

    QPixmap pixmap(22, 22);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    color.setAlpha(255);
    p.setBrush(color);
    p.drawRect(2, 2, 18, 18);
    if (!chr.isNull()) {
        QFont f;
        f.setBold(true);
        f.setPixelSize(18);
        p.setFont(f);
        // p.setPen(Qt::white);
        p.drawText(QRect(2, 2, 18, 18), Qt::AlignCenter, {chr});
    }
    return pixmap;
}

using LayerTypes = std::vector<LayerType>;

namespace FileTree {
class Node;
}

class AbstractFile {

    friend QDataStream& operator<<(QDataStream& stream, const AbstractFile& file) {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        Block(out).write(
            file.id_,
            file.date_,
            file.groupedPaths_,
            file.itemsType_,
            file.lines_,
            file.mergedPaths_,
            file.name_,
            file.side_,
            file.transform_,
            file.isVisible(),
            file.color_,
            file.colorFlag_);
        file.write(out);
        return stream << data;
    }

    friend QDataStream& operator>>(QDataStream& stream, AbstractFile& file) {
        QByteArray data;
        stream >> data;
        QDataStream in(&data, QIODevice::ReadOnly);
        bool visible;
        Block(in).read(
            file.id_,
            file.date_,
            file.groupedPaths_,
            file.itemsType_,
            file.lines_,
            file.mergedPaths_,
            file.name_,
            file.side_,
            file.transform_,
            visible,
            file.color_,
            file.colorFlag_);
        file.read(in);
        if (App::splashScreen())
            App::splashScreen()->showMessage(QObject::tr("Preparing: ") + file.shortName() + "\n\n\n", Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
        file.createGi();
        file.setTransform(file.transform_);
        file.setVisible(visible);
        return stream;
    }

public:
    template <typename T>
    static inline T* load(QDataStream& stream) {
        auto* file = new T;
        stream >> *file;
        return file;
    }

    AbstractFile();

    virtual ~AbstractFile();

    QString shortName() const;
    QString name() const;
    void setFileName(const QString& fileName);

    void addToScene() const;

    GiGroup* itemGroup(int type = -1) const;

    const mvector<GiGroup*>& itemGroups() const;

    Paths mergedPaths() const;
    Pathss groupedPaths() const;

    mvector<QString>& lines();
    const mvector<QString>& lines() const;
    const QString lines2() const;
    virtual mvector<const GraphicObject*> graphicObjects() const;

    enum Group {
        CopperGroup,
        CutoffGroup,
    };

    const LayerTypes& displayedTypes() const;
    Side side() const;
    void setSide(Side side);

    virtual mvector<const GraphicObject*> getDataForGC(std::span</*GraphicObject::Type*/ int>, Range area = {}, Range length = {}) const { return {}; };
    virtual void initFrom(AbstractFile* file);
    virtual uint32_t type() const = 0;
    virtual void createGi() = 0;
    virtual void setItemType([[maybe_unused]] int type);
    virtual int itemsType() const;

    virtual bool isVisible() const;
    virtual void setVisible(bool visible);
    const QColor& color() const;
    virtual void setColor(const QColor& color);

    virtual FileTree::Node* node() = 0;
    virtual QIcon icon() const = 0;

    void setTransform([[maybe_unused]] const Transform& transform);
    const Transform& transform() const;

    int32_t id() const;
    void setId(int32_t id);

    bool userColor() const;
    void setUserColor(bool userColor);

protected:
    virtual void write(QDataStream& stream) const = 0;
    virtual void read(QDataStream& stream) = 0;
    virtual Paths merge() const = 0;

    LayerTypes layerTypes_;
    FileTree::Node* node_ = nullptr;
    Pathss groupedPaths_;
    QColor color_;
    bool colorFlag_ {};
    QDateTime date_;
    QString name_;
    Side side_ = Top;
    int32_t id_ = -1;
    int itemsType_ = -1;
    mutable Paths mergedPaths_;
    mutable bool visible_ = false;
    mvector<GiGroup*> itemGroups_;
    mvector<QString> lines_;
    //    QTransform transform_;
    Transform transform_;
};

inline AbstractFile::AbstractFile()
    : itemGroups_ {new GiGroup} {
}

inline AbstractFile::~AbstractFile() { qDeleteAll(itemGroups_); }

inline QString AbstractFile::shortName() const { return QFileInfo(name_).fileName(); }

inline QString AbstractFile::name() const { return name_; }

inline void AbstractFile::setFileName(const QString& fileName) { name_ = fileName; }

inline void AbstractFile::addToScene() const {
    for (const auto var : itemGroups_) {
        if (var && var->size()) {
            var->addToScene();
            var->setZValue(-id_);
        }
    }
}

inline GiGroup* AbstractFile::itemGroup(int type) const {
    const int size(static_cast<int>(itemGroups_.size()));
    if (type == -1 && 0 <= itemsType_ && itemsType_ < size)
        return itemGroups_[itemsType_];
    else if (0 <= type && type < size)
        return itemGroups_[type];
    return itemGroups_.front();
}

inline const mvector<GiGroup*>& AbstractFile::itemGroups() const { return itemGroups_; }

inline Paths AbstractFile::mergedPaths() const { return mergedPaths_.size() ? mergedPaths_ : merge(); }

inline Pathss AbstractFile::groupedPaths() const { return groupedPaths_; }

inline mvector<QString>& AbstractFile::lines() { return lines_; }

inline const mvector<QString>& AbstractFile::lines() const { return lines_; }

inline const QString AbstractFile::lines2() const {
    QString rstr;
    for (auto&& str : lines_)
        rstr.append(str).append('\n');
    return rstr;
}

inline mvector<const GraphicObject*> AbstractFile::graphicObjects() const { return {}; }

inline void AbstractFile::initFrom(AbstractFile* file) {
    id_ = file->id_;
    node_ = file->node_;
    side_ = file->side_;
    colorFlag_ = file->colorFlag_;
    setColor(file->color_);
    setItemType(file->itemsType_);
    setTransform(file->transform_);
    for (auto* ig : itemGroups_)
        for (auto* gi : *ig)
            gi->setZValue(-id_);
}

inline const LayerTypes& AbstractFile::displayedTypes() const { return layerTypes_; }

inline void AbstractFile::setItemType(int type) { }

inline int AbstractFile::itemsType() const { return itemsType_; }

inline Side AbstractFile::side() const { return side_; }

inline void AbstractFile::setSide(Side side) { side_ = side; }

inline bool AbstractFile::isVisible() const { return (visible_ = itemGroup()->isVisible()); }

inline void AbstractFile::setVisible(bool visible) { itemGroup()->setVisible(visible_ = visible); }

inline const QColor& AbstractFile::color() const { return color_; }

inline void AbstractFile::setColor(const QColor& color) { color_ = color; }

inline void AbstractFile::setTransform(const Transform& transform) {
    transform_ = transform;
    QTransform t {transform};
    for (auto* ig : itemGroups_)
        for (auto* gi : *ig)
            gi->setTransform(t);
}

inline const Transform& AbstractFile::transform() const { return transform_; }

inline int AbstractFile::id() const { return id_; }

inline void AbstractFile::setId(int32_t id) { id_ = id; }

inline bool AbstractFile::userColor() const { return colorFlag_; }

inline void AbstractFile::setUserColor(bool userColor) { colorFlag_ = userColor; }

class TransformDialog : public QDialog {
    Q_OBJECT
    enum { Ang,
        TrX,
        TrY,
        ScX,
        ScY };
    DoubleSpinBox* dsbx[5];
    std::vector<AbstractFile*> files_;
    std::unordered_map<AbstractFile*, Transform> backup;

public:
    TransformDialog(std::vector<AbstractFile*>&& files, QWidget* parent = nullptr)
        : QDialog {parent}
        , files_ {std::move(files)} {

        setMaximumSize(0, 0);
        setWindowTitle(tr("Affine Transform"));

        for (auto& dsbx : dsbx)
            dsbx = new DoubleSpinBox(this);

        dsbx[Ang]->setRange(-360, +360);
        dsbx[Ang]->setSuffix(tr(" °"));
        dsbx[ScX]->setRange(-10, +10);
        dsbx[ScY]->setRange(-10, +10);
        dsbx[TrX]->setRange(-1000, +1000);
        dsbx[TrX]->setSuffix(tr(" mm"));
        dsbx[TrY]->setRange(-1000, +1000);
        dsbx[TrY]->setSuffix(tr(" mm"));

        // QPushButton button(tr("Apply"), &d);
        // layout.addRow(new QWidget(&d), &button);
        resize({0, 0});

        auto transform = files_.front()->transform();

        dsbx[Ang]->setValue(transform.angle);
        dsbx[TrX]->setValue(transform.translate.x());
        dsbx[TrY]->setValue(transform.translate.y());
        dsbx[ScX]->setValue(transform.scale.x());
        dsbx[ScY]->setValue(transform.scale.y());

        for (auto* file : files_)
            backup.emplace(file, file->transform());

        for (auto& dsbx : dsbx)
            connect(dsbx, &QDoubleSpinBox::valueChanged, this, &TransformDialog::setTransform);

        auto buttonBox = new QDialogButtonBox(this);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(30, 240, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &TransformDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &TransformDialog::reject);

        auto layout = new QFormLayout(this);
        layout->addRow(new QLabel(tr("Angle:"), this), dsbx[Ang]);
        layout->addRow(new QLabel(tr("Translate X:"), this), dsbx[TrX]);
        layout->addRow(new QLabel(tr("Translate Y:"), this), dsbx[TrY]);
        layout->addRow(new QLabel(tr("Scale X:"), this), dsbx[ScX]);
        layout->addRow(new QLabel(tr("Scale Y:"), this), dsbx[ScY]);
        layout->setLabelAlignment(Qt::AlignRight);
        layout->addRow(buttonBox);

        QMetaObject::connectSlotsByName(this);
    }

    ~TransformDialog() override = default;

    void setTransform() {
        Transform transform {
            .angle = dsbx[Ang]->value(),
            .translate {dsbx[TrX]->value(), dsbx[TrY]->value()},
            .scale {dsbx[ScX]->value(), dsbx[ScY]->value()},
        };
        for (auto* file : files_)
            file->setTransform(transform);
    };

    void reject() override {
        for (auto&& [file, transform] : backup)
            file->setTransform(transform);
        QDialog::reject();
    }
};

#define FileInterface_iid "ru.xray3d.XrSoft.GGEasy.AbstractFile"

Q_DECLARE_INTERFACE(AbstractFile, FileInterface_iid)

Q_DECLARE_METATYPE(mvector<const GraphicObject*>)
