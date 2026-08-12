/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_creator.h"
#include "gc_file.h"

#include <QIcon>
#include <QPixmap>

namespace Profile {

struct Settings {
    int sort{};
};

inline Settings settings;

inline constexpr auto PROFILE = "Profile"_hash32;

class[[= Serial::name("Profile")]] File final : public GCode::File {
public:
    void serialize(Serial::Writer& sb) const override { Serial::writeInto(sb, *this); }
    explicit File();
    explicit File(GCode::Params&& gcp);
    QIcon icon() const override { return QIcon::fromTheme(u"profile-path"_s); }
    uint32_t type() const override { return PROFILE; }
    void createGi() override;
    void genGcodeAndTile() override;

private:
    // Тот же профиль, но с мостиками (табами): контур остаётся одним
    // непрерывным проходом, а над мостами фреза приподнимается горбом до
    // верха таба -- см. подробный комментарий у реализации в profile.cpp.
    void saveMillingProfileBridges(const QPointF& offset);
}; // File

class Creator : public GCode::Creator {

public:
    Creator() = default;
    ~Creator() override = default;

    // Мостики: длина перемычки и её высота от дна реза. Сами мосты приезжают
    // центрами в gcp.supportCurvess -- см. Form::computePaths.
    static inline const QString BridgeLen = u"BridgeLen"_s;
    static inline const QString BridgeHeight = u"BridgeHeight"_s;
    static inline const QString TrimmingCorners = u"TrimmingCorners"_s;
    static inline const QString TrimmingOpenPaths = u"TrimmingOpenPaths"_s;
    static inline const QString Allowance = u"Allowance"_s;

private:
    void createProfile(const Tool& tool, const double depth);

    // Порядок обхода и направление фрезерования. Вынесено из reorder(), потому
    // что черновой и чистовой наборы обязаны обходиться одинаково -- иначе
    // проходы не будут соответствовать друг другу.
    Geo::Polylines orderContours(Geo::Polylines contours);

    // Чистовой проход по целевому контуру -- пока такой же, как черновой, и
    // дописывается в те же returnPss. Касательный подвод/отвод отложен, см.
    // #if 0 в начале profile.cpp.
    void makeFinishing();

    // Укорачивает открытые пути на радиус фрезы с каждого конца: инструмент
    // круглый, и на самом конце линии он бы залез за неё. Путь короче
    // диаметра после такой обрезки не остался бы вовсе -- он выбрасывается.
    void trimmingOpenPaths(Geo::Polylines& paths);

    // «Собачьи кости» во внутренних прямых углах: круглая фреза угол не
    // выбирает, и в него добавляется нырок по биссектрисе.
    void cornerTrimming();

    // Порядок обхода контуров по вложенности и направление фрезерования.
    void reorder();

protected:
    void create() override; // Creator interface
    uint32_t type() override { return PROFILE; }
    bool possibleTest() const override { return true; }
};

} // namespace Profile
