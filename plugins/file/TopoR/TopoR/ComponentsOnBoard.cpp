#include "ComponentsOnBoard.h"
namespace TopoR {
bool ComponentsOnBoard::CompInstance::Pin::ShouldSerializePadstackRef() { return PadstackRef != TopoR::PadstackRef{}; }

bool ComponentsOnBoard::CompInstance::Attribute::ShouldSerialize_Labels() { return Labels.size(); }

bool ComponentsOnBoard::CompInstance::ShouldSerialize_Pins() { return Pins.size(); }
bool ComponentsOnBoard::CompInstance::ShouldSerialize_Mntholes() { return Mntholes.size(); }
bool ComponentsOnBoard::CompInstance::ShouldSerialize_Attributes() { return Attributes.size(); }
std::string ComponentsOnBoard::CompInstance::ToString() { return name; }

bool ComponentsOnBoard::ShouldSerialize_Components() { return Components.size(); }
bool ComponentsOnBoard::ShouldSerialize_FreePads() { return FreePads.size(); }
std::string ComponentsOnBoard::AddComponent(const std::string& name, units units, const std::string& componentRef, const std::string& footprintRef) {
    //    double x = 0, y{}; // координаты нового компонента
    //    if(Components.empty())
    //        return L"";
    //    while(ComponentIndexOf(name) >= 0) // проверка на уникальность имени и добавление префикса
    //        name += L"_";
    //    for(int i = Components.size(); i > 0; i--) // вычисление максимально возможных координат
    //    {
    //        x = std::max(x, Components[i - 1]->Org.x);
    //        y = std::max(y, Components[i - 1]->Org.y);
    //    }
    //    double offset = units == units::mm ? 3 : Ut::UnitsConvert(3, dist::mm, dist::mil);
    //    x += offset; // добавление небольшого смещения
    //    y += offset;
    //    Org tempVar = std::make_shared<Org>();
    //    tempVar->x = x;
    //    tempVar->y = y;
    //    ComponentRef tempVar2 = std::make_shared<ComponentRef>();
    //    tempVar2->ReferenceName = componentRef;
    //    FootprintRef tempVar3 = std::make_shared<FootprintRef>();
    //    tempVar3->ReferenceName = footprintRef;
    //    CompInstance c = {.name = name, .side = side::Top, .uniqueId = UniqueId(), .angle = 0, .fixed = Bool::off, .Org = tempVar, .ComponentRef = tempVar2, .FootprintRef = tempVar3};
    //    Components.push_back(c);
    return {}; //    return c->name;
}
bool ComponentsOnBoard::RemoveComponent(const std::string& name) {
    //    int x = ComponentIndexOf(name);
    //    if(x >= 0) {
    //        Components.erase(Components.begin() + x);
    //        return true;
    //    }
    return {}; //    return false;
}
int ComponentsOnBoard::ComponentIndexOf(const std::string& name) {
    //    for(int x = Components.empty() ? nullptr : ((Components.size() != nullptr) ? Components.size() : 0); x > 0; x--)
    //        if(Components[x - 1]->name == name)
    //            return x - 1;
    return {}; //    return -1;
}
int ComponentsOnBoard::RenameComponent(const std::string& oldname, const std::string& newname) {
    //    int x = ComponentIndexOf(oldname);
    //    if(x >= 0) {
    //        Components[x]->name = newname;
    //        return x;
    //    }
    return {}; //    return -1;
}
std::string ComponentsOnBoard::UniqueId() {
    //    std::string ABC = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    //    std::string uniqueId = L"";
    //    Random rnd = std::make_shared<Random>();
    //    for(int i{}; i < 8; i++)
    //        uniqueId += ABC[rnd->Next(26)];
    //    {
    //        for(auto c: Components)
    //            if(c->uniqueId == uniqueId)
    //                uniqueId = UniqueId();
    //    }
    return {}; //    return uniqueId;
}
} // namespace TopoR
