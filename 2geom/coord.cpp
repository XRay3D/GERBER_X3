/** @file
 * @brief Conversion between Coord and strings
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2014 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

// Most of the code in this file is derived from:
// https://code.google.com/p/double-conversion/
// The copyright notice for that code is attached below.
//
// Copyright 2010 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <2geom/coord.h>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <climits>
#include <cstdarg>
#include <cmath>

#include <double-conversion/double-conversion.h>

namespace Geom {

std::string format_coord_shortest(Coord x)
{
    static const double_conversion::DoubleToStringConverter conv(
        double_conversion::DoubleToStringConverter::UNIQUE_ZERO,
        "inf", "NaN", 'e', -3, 6, 0, 0);
    std::string ret(' ', 32);
    double_conversion::StringBuilder builder(&ret[0], 32);
    conv.ToShortest(x, &builder);
    ret.resize(builder.position());
    return ret;
}

std::string format_coord_nice(Coord x)
{
    static const double_conversion::DoubleToStringConverter conv(
    	double_conversion::DoubleToStringConverter::UNIQUE_ZERO,
        "inf", "NaN", 'e', -6, 21, 0, 0);
    std::string ret(' ', 32);
    double_conversion::StringBuilder builder(&ret[0], 32);
    conv.ToShortest(x, &builder);
    ret.resize(builder.position());
    return ret;
}

Coord parse_coord(std::string const &s)
{
    static const double_conversion::StringToDoubleConverter conv(
    	double_conversion::StringToDoubleConverter::ALLOW_LEADING_SPACES |
		double_conversion::StringToDoubleConverter::ALLOW_TRAILING_SPACES |
		double_conversion::StringToDoubleConverter::ALLOW_SPACES_AFTER_SIGN,
        0.0, nan(""), "inf", "NaN");
    int dummy;
    return conv.StringToDouble(s.c_str(), s.length(), &dummy);
}

} // namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
