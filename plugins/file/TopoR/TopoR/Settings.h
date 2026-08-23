#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Настройки дизайна».
struct Settings {
    // Настройки автоматической трассировки.
    struct Autoroute {
        // Настройка автоматической трассировки: режим трассировки.
        [[= XML::Attr]] AutorouteMode mode{};
        // Параметр автоматической трассировки: использование функциональной эквивалентности.
        [[= XML::Attr]] autoEqu autoEqu{};
        // Параметр автоматической трассировки: форма проводников.
        [[= XML::Attr]] wireShape wireShape{};
        // Параметр автоматической трассировки: создавать «капельки».
        [[= XML::Attr]] Bool teardrops{};
        // public bool teardropsSpecified
        bool getTeardropsSpecified() const;
        // Параметр автоматической трассировки: ослабленный контроль зазоров.
        [[= XML::Attr]] Bool weakCheck{};
        // public bool weakCheckSpecified
        bool getWeakCheckSpecified() const;
        // Параметр автоматической трассировки: использовать имеющуюся разводку в качестве начального варианта.
        [[= XML::Attr]] Bool takeCurLayout{};
        // public bool takeCurLayoutSpecified
        bool getTakeCurLayoutSpecified() const;
        // Настройка автоматической трассировки: соединять планарные контакты напрямую.
        [[= XML::Attr]] Bool directConnectSMD{};
        // public bool directConnectSMDSpecified
        bool getDirectConnectSMDSpecified() const;
        // Настройка автоматической трассировки: не дотягивать проводник до точки привязки полигонального контакта.
        [[= XML::Attr]] Bool dontStretchWireToPolypin{};
        // public bool dontStretchWireToPolypinSpecified
        bool getDontStretchWireToPolypinSpecified() const;
    };
    // Настройки автоматических процедур.
    struct Autoproc {
        // Настройка автоматической перекладки проводников.
        [[= XML::Attr]] refine refine{};
        // Настройка автоматической подвижки.
        [[= XML::Attr]] automove automove{};
    };
    // Настройки автоматического размещения компонентов.
    struct Placement {
        // Настройки автоматического размещения компонентов: область размещения. Область прямоугольная, задаётся двумя вершинами(верхняя левая и правая нижняя).
        struct PlacementArea {
            // Координаты точек, вершин
            ///*[[= XML::Elem]]*/ // public List<Dot> Dots;
            [[= XML::Elem]] std::vector<Dot> Dots;
            bool ShouldSerialize_Dots();
        };
        // Настройки автоматического размещения компонентов: область размещения. Область прямоугольная, задаётся двумя вершинами(верхняя левая и правая нижняя).
        /*[[= XML::Elem]]*/ PlacementArea PlacementArea;
    };
    // Настройки ориентации ярлыков.
    struct Labels {
        // Настройка ориентации ярлыков: вращать ярлык при вращении компонента.
        [[= XML::Attr]] Bool rotateWithComp{};
        // public bool rotateWithCompSpecified
        bool getRotateWithCompSpecified() const;
        // Настройка редактирования ярлыков: использовать правила ориентации.
        [[= XML::Attr]] Bool useOrientRules{};
        // public bool useOrientRulesSpecified
        bool getUseOrientRulesSpecified() const;
        // Настройка ориентации ярлыков: поворот для ярлыков горизонтальной ориентации на верхней стороне.
        [[= XML::Attr]] Bool topHorzRotate{};
        // public bool topHorzRotateSpecified
        bool getTopHorzRotateSpecified() const;
        // Настройка ориентации ярлыков: поворот для ярлыков вертикальной ориентации на верхней стороне.
        [[= XML::Attr]] Bool topVertRotate{};
        // public bool topVertRotateSpecified
        bool getTopVertRotateSpecified() const;
        // Настройка ориентации ярлыков: поворот для ярлыков горизонтальной ориентации на нижней стороне.
        [[= XML::Attr]] Bool bottomHorzRotate{};
        // public bool bottomHorzRotateSpecified
        bool getBottomHorzRotateSpecified() const;
        // Настройка ориентации ярлыков: поворот для ярлыков вертикальной ориентации на нижней стороне.
        [[= XML::Attr]] Bool bottomVertRotate{};
        // public bool bottomVertRotateSpecified
        bool getBottomVertRotateSpecified() const;
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Настройки автоматической трассировки.
    /*[[= XML::Elem]]*/ Autoroute Autoroute;
    // Настройки автоматических процедур.
    /*[[= XML::Elem]]*/ Autoproc Autoproc;
    // Настройки автоматического размещения компонентов.
    /*[[= XML::Elem]]*/ Placement Placement;
    // Настройки ориентации ярлыков.
    /*[[= XML::Elem]]*/ Labels Labels;
    /********************************************************************
     * Здесь находятся функции для работы с элементами класса Settings. *
     * Они не являются частью формата TopoR PCB.                        *
     * ******************************************************************/
    /********************************************************************/
};
} // namespace TopoR
