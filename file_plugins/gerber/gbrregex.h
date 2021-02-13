/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#ifdef CTRE

#include <ctre.hpp> //

[[maybe_unused]] static constexpr auto ptrnAperture = ctll::fixed_string("^%ADD(\\d\\d+)([a-zA-Z_$\\.][a-zA-Z0-9_$\\.\\-]*),?(.*)\\*%$");
[[maybe_unused]] static constexpr auto ptrnApertureBlock = ctll::fixed_string("^%ABD(\\d+)\\*%$");
[[maybe_unused]] static constexpr auto ptrnApertureMacros = ctll::fixed_string("^%AM([^\\*]+)\\*([^%]+)?(%)?$");
[[maybe_unused]] static constexpr auto ptrnAttributes = ctll::fixed_string("^%(T[FAOD])(\\.?)(.*)\\*%$");
[[maybe_unused]] static constexpr auto ptrnDCode = ctll::fixed_string("^D0?([123])\\*$");
[[maybe_unused]] static constexpr auto ptrnDCodeAperture = ctll::fixed_string("^(?:G54)?D(\\d+)\\*$");
[[maybe_unused]] static constexpr auto ptrnDummy = ctll::fixed_string("^%(.{2})(.+)\\*%$");
[[maybe_unused]] static constexpr auto ptrnEndOfFile1 = ctll::fixed_string("^M[0]?[0123]\\*");
[[maybe_unused]] static constexpr auto ptrnEndOfFile2 = ctll::fixed_string("^D0?2M0?[02]\\*");
[[maybe_unused]] static constexpr auto ptrnFormat = ctll::fixed_string("^%FS([LT]?)([AI]?)X(\\d)(\\d)Y(\\d)(\\d)\\*%$");
[[maybe_unused]] static constexpr auto ptrnGCode = ctll::fixed_string("^G([0]?[0-9]{2})\\*$");
[[maybe_unused]] static constexpr auto ptrnGCodeComment = ctll::fixed_string("^G0?4(.*)$");
[[maybe_unused]] static constexpr auto ptrnImagePolarity = ctll::fixed_string("^%IP(POS|NEG)\\*%$");
[[maybe_unused]] static constexpr auto ptrnLoadName = ctll::fixed_string("^%LN(.+)\\*%$");
[[maybe_unused]] static constexpr auto ptrnPosition = ctll::fixed_string("(?:G[01]{1,2})?(?:X([\\+\\-]?\\d*\\.?\\d+))?(?:Y([\\+\\-]?\\d*\\.?\\d+))?.+");
[[maybe_unused]] static constexpr auto ptrnStepRepeat = ctll::fixed_string("^%SRX(\\d+)Y(\\d+)I(.\\d*\\.?\\d*)J(.\\d*\\.?\\d*)\\*%$");
[[maybe_unused]] static constexpr auto ptrnStepRepeatEnd = ctll::fixed_string("^%SR\\*%$");
[[maybe_unused]] static constexpr auto ptrnTransformations = ctll::fixed_string("^%L([PMRS])(.+)\\*%$");
[[maybe_unused]] static constexpr auto ptrnUnitMode = ctll::fixed_string("^%MO(IN|MM)\\*%$");
[[maybe_unused]] static constexpr auto ptrnCircularInterpolation = ctll::fixed_string("^(?:G0?([23]))?"
                                                                                      "X?([\\+\\-]?\\d+)*"
                                                                                      "Y?([\\+\\-]?\\d+)*"
                                                                                      "I?([\\+\\-]?\\d+)*"
                                                                                      "J?([\\+\\-]?\\d+)*"
                                                                                      "[^D]*(?:D0?([12]))?\\*$");
[[maybe_unused]] static constexpr auto ptrnLineInterpolation = ctll::fixed_string("^(?:G0?(1))?(?=.*X([\\+\\-]?\\d+))?(?=.*Y([\\+\\-]?\\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\\*$");

[[maybe_unused]] constexpr auto reAperture = ctre::match<ptrnAperture>;
[[maybe_unused]] constexpr auto reApertureBlock = ctre::match<ptrnApertureBlock>;
[[maybe_unused]] constexpr auto reApertureMacros = ctre::match<ptrnApertureMacros>;
[[maybe_unused]] constexpr auto reAttributes = ctre::match<ptrnAttributes>;
[[maybe_unused]] constexpr auto reDCode = ctre::match<ptrnDCode>;
[[maybe_unused]] constexpr auto reDCodeAperture = ctre::match<ptrnDCodeAperture>;
[[maybe_unused]] constexpr auto reDummy = ctre::match<ptrnDummy>;
[[maybe_unused]] constexpr auto reEndOfFile1 = ctre::match<ptrnEndOfFile1>;
[[maybe_unused]] constexpr auto reEndOfFile2 = ctre::match<ptrnEndOfFile2>;
[[maybe_unused]] constexpr auto reFormat = ctre::match<ptrnFormat>;
[[maybe_unused]] constexpr auto reGCode = ctre::match<ptrnGCode>;
[[maybe_unused]] constexpr auto reGCodeComment = ctre::match<ptrnGCodeComment>;
[[maybe_unused]] constexpr auto reImagePolarity = ctre::match<ptrnImagePolarity>;
[[maybe_unused]] constexpr auto reLoadName = ctre::match<ptrnLoadName>;
[[maybe_unused]] constexpr auto rePosition = ctre::match<ptrnPosition>;
[[maybe_unused]] constexpr auto reStepRepeat = ctre::match<ptrnStepRepeat>;
[[maybe_unused]] constexpr auto reStepRepeatEnd = ctre::match<ptrnStepRepeatEnd>;
[[maybe_unused]] constexpr auto reTransformations = ctre::match<ptrnTransformations>;
[[maybe_unused]] constexpr auto reUnitMode = ctre::match<ptrnUnitMode>;
[[maybe_unused]] constexpr auto reCircularInterpolation = ctre::match<ptrnCircularInterpolation>;
[[maybe_unused]] constexpr auto reLineInterpolation = ctre::match<ptrnLineInterpolation>;

#endif
