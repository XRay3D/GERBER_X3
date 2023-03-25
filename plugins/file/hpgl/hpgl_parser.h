/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     * * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "hpgl_types.h"

class FileInterface;
class FilePlugin;

namespace Hpgl {

class File;

class Parser {
    Q_GADGET

    FilePlugin* const interface;

public:
    Parser(FilePlugin* const interface = nullptr);
    FileInterface* parseFile(const QString& fileName);

    enum Cmd {
        /*
        IN, //         IN;	инициализация процесса черчения
        IP, //         IP;	определяет начальную точку, в данном случае по умолчанию 0,0
        SC, //         SC0,100,0,100;	устанавливает размеры страницы от 0 до 100 в направлениях X и Y
        SP, //         SP1;	выбирает перо 1
        PU, //         PU0,0;	перемещает перо в начальную позицию
        PD, //         PD100,0,100,100,0,100,0,0;	опускает и двигает перо по заданным позициям (чертит прямоугольник вокруг страницы)
        PU, //         PU50,50;	поднимает и перемещает перо в позицию 50,50
        CI, //         CI25;	чертит окружность с радиусом 25
        SS, //         SS;	выбирает стандартный шрифт
        DT, //         DT*,1;	устанавливает в качестве текстового разделителя символ * и запрещает его печать на бумаге (1 — «true»)
        PU, //         PU20,80;	поднимает и перемещает перо в позицию 20,80
        LB, //         LBHello World*;	чертит надпись
        */
        //        Initialization and default setting instructions
        //        Instruction Function page
        DF, //         DF Default Set Instruction 6
        IN, //         IN Initialize Set Instruction 7
        //        Plot area and unit setting instructions
        //        Instruction Function page
        IP, //         IP Scaling point 8
        SC, //         SC Scale 8
        IW, //         IW Input window 9
        RO, //         RO Rotate coordinate system 9
        PG, //         PG Page output 9
        //        Pen Control and Plot Instructions
        //        Instruction Function page
        PU, //         PU Pen Up 10
        PD, //         PD Pen Down 11
        PA, //         PA Plot Absolute 12
        PR, //         PR Relative Coordinate Pen Move 13
        AA, //         AA Absolute Arc Plot 14
        AR, //         AR Relative Arc Plot 15
        CI, //         CI Circle 16
        //        The polygon group
        //        Instruction Function page
        EA, //         EA Edge Absolute Rectangle 18
        ER, //         ER Edge Relative Rectangle 19
        EW, //         EW Edge Wedge 20
        RA, //         RA Fill Absolute Rectangle 21
        RR, //         RR Fill Relative Rectangle 22
        WG, //         WG Fill Wedge 23
        //        Plot Function Instructions
        //        Instruction Function page
        FT, //         FT Fill Type 24
        LT, //         LT Line Type 25
        PW, //         PW Pen Width 25
        SM, //         SM Symbol Mode 26
        SP, //         SP Select Pen 26
        TL, //         TL Tick Length 26
        XT, //         XT X Tick 27
        YT, //         YT Y Tick 27
        PT, //         PT Pen Thickness 27
        //        Character Plot Instructions
        //        Instruction Function page
        CS, //         CS Standard Set Definition 28
        CA, //         CA Alternate Set Definition 28
        SS, //         SS Select Standard Font 29
        SA, //         SA Select Alternate Font 29
        DT, //         DT Define Label Terminator 29
        LB, //         LB Define Label 30
        DI, //         DI Absolute Direction 30
        DR, //         DR Relative Direction 31
        CP, //         CP Character Plot 31
        SI, //         SI Set Absolute Character Size 32
        SR, //         SR Set Relative Character Size 33
        SL, //         SL Set Character Slant 33
        UC, //         UC User-defined Character 34
        //        Revision C 29.10.99
        //        Chapter 9 "HP-GL" 3
        //        Dual Context Extensions
        //        Instruction Function page
        //        EscCRRO Set HRC Off 35
        //        EscCRRL Set HRC to Light Level 35
        //        EscCRRM Set HRC to Medium Level 35
        //        EscCRRD Set HRC to Dark Level 34
        //        User Reset
        //        Instruction Function page
        //        EscCR!#R Restore to User Settings 35
        //        Factory Reset
        //        Instruction Function page
        //        EscCRFD Restore to Factory Settings 35
    };
    Q_ENUM(Cmd)
    static Cmd toCmd(const QStringRef& str);

protected:
    File* file = nullptr;
    //    State m_state;
};

} // namespace Hpgl
