#include "ComponentsOnBoard.h"

#if COMPONENTSONBOARD

namespace TopoR {

QString ComponentsOnBoard::AddComponent(QString name, units units, const QString& componentRef, const QString& footprintRef) {
    if(Components.empty()) return {};

    while(ComponentIndexOf(name) >= 0) // проверка на уникальность имени и добавление суффикса
        name += "_";

    double x{}, y{}; // координаты нового компонента

    for(int i = Components.size(); i > 0; --i) { // вычисление максимально возможных координат
        x = std::max<double>(x, Components[i - 1].org.x);
        y = std::max<double>(y, Components[i - 1].org.y);
    }

    double offset = (units == units::mm) ? 3 : 3 * Ut::UnitsConvert(dist::mm, dist::mil);
    x += offset; // добавление небольшого смещения
    y += offset;

    CompInstance c{
        .name{name},
        .uniqueId{UniqueId()},
        .side_{side::Top},
        .angle{},
        .fixed{/*Bool::off*/},
        .componentRef{componentRef},
        .footprintRef{footprintRef},
        .org{{x}, {y}},
        .Pins{},
        .Mntholes{},
        .Attributes{}
    };

    return Components.emplace_back(std::move(c)).name;
}

QTransform ComponentsOnBoard::CompInstance::Attribute::Label::transform() const {
    QTransform transform;
    transform.translate(org.x, org.y);
    transform.rotate(angle);
    return transform;
}

QTransform ComponentsOnBoard::FreePad::transform() const {
    QTransform transform;
    transform.translate(org.x, org.y);
    transform.rotate(angle);
    return transform;
}

bool ComponentsOnBoard::RemoveComponent(const QString& name) {
    auto it = std::ranges::find(Components, name, &CompInstance::name);
    return (it != Components.end()) ? Components.erase(it), true : false;
}

int ComponentsOnBoard::ComponentIndexOf(const QString& name) {
    auto it = std::ranges::find(Components, name, &CompInstance::name);
    return (Components.end() == it) ? distance(Components.begin(), it) : -1;
}

int ComponentsOnBoard::RenameComponent(const QString& oldName, const QString& newName) {
    int x = ComponentIndexOf(oldName);
    if(x >= 0) {
        Components[x].name = newName;
        return x;
    }
    return -1;
}

QString ComponentsOnBoard::UniqueId() {
    char ABC[]{"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    QString uniqueId;
    while(uniqueId.size() < 8)
        uniqueId += ABC[rand() % 26];
    if(Components.size()) {
        for(auto&& component: Components)
            if(component.uniqueId == uniqueId)
                uniqueId = UniqueId();
    }
    return uniqueId;
}

QTransform ComponentsOnBoard::CompInstance::transform() const {
    QTransform transform;
    transform.translate(org.x, org.y);
    if(side_ == side::Bottom) transform.scale(-1, 1);
    transform.rotate(angle); // FIXME
    return transform;
}

} // namespace TopoR

#endif
