/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "dxf_entity.h"
namespace Dxf {
struct Attrib final : Entity {
    Attrib(SectionParser* sp);

    // Entity interface
public:
    void parse(CodeData& code) override;
    Type type() const override { return ATTRIB; };
    GraphicObject toGo() const override;

    /*
Attrib group codes

Group code


Description

100


Subclass marker (AcDbText)

39


Thickness (optional; default = 0)

10


Text start point (in OCS)

DXF: X value; APP: 3D point

20, 30


DXF: Y and Z values of text start point (in OCS)

40


Text height

1


Default value (string)

100


Subclass marker (AcDbAttribute)

280


Version number:

0 = 2010

2


Attribute tag (string; cannot contain spaces)

70


Attribute flags:

1 = Attribute is invisible (does not appear)

2 = This is a constant attribute

4 = Verification is required on input of this attribute

8 = Attribute is preset (no prompt during insertion)

73


Field length (optional; default = 0) (not currently used)

50


Text rotation (optional; default = 0)

41


Relative X scale factor (width) (optional; default = 1). This value is also adjusted when fit-type text is used

51


Oblique angle (optional; default = 0)

7


Text style name (optional; default = STANDARD)

71


Text generation flags (optional; default = 0). See TEXT group codes

72


Horizontal text justification type (optional; default = 0). See TEXT group codes

74


Vertical text justification type (optional; default = 0). See group code 73 in TEXT

11


Alignment point (in OCS) (optional)

DXF: X value; APP: 3D point

Present only if 72 or 74 group is present and nonzero

21, 31


DXF: Y and Z values of alignment point (in OCS) (optional)

210


Extrusion direction. Present only if the entity's extrusion direction is not parallel to the WCS Z axis (optional; default = 0, 0, 1)

DXF: X value; APP: 3D vector

220, 230


DXF: Y and Z values of extrusion direction (optional)

280


Lock position flag. Locks the position of the attribute within the block reference

100


Subclass marker (AcDbXrecord)

280


Duplicate record cloning flag (determines how to merge duplicate entries):

1 = Keep existing

70


MText flag:

2 = multiline attribute

4 = constant multiline attribute definition

70


isReallyLocked flag:

0 = unlocked

1 = locked

70


Number of secondary attributes or attribute definitions

340


Hard-pointer id of secondary attribute(s) or attribute definition(s)

10


Alignment point of attribute or attribute definition

DXF: X value; APP: 3D point

20,30


DXF: Y and Z values of insertion point

40


current annotation scale

2


attribute or attribute definition tag string

0


Entity type (MTEXT)

100


Subclass marker (AcDbEntity)

67


Absent or zero indicates entity is in model space. 1 indicates entity is in paper space (optional)

8


Layer name

100


Subclass marker (AcDbMText)

10


Insertion point

DXF: X value; APP: 3D point

20,30


DXF: Y and Z values of insertion point

40


Nominal (initial) text height

41


Reference rectangle width

46


Defined annotation height

71


Attachment point:

1 = Top left; 2 = Top center; 3 = Top right

4 = Middle left; 5 = Middle center; 6 = Middle right

7 = Bottom left; 8 = Bottom center; 9 = Bottom right

72


Drawing direction:

1 = Left to right

3 = Top to bottom

5 = By style (the flow direction is inherited from the associated text style)

1


Text string

If the text string is less than 250 characters, all characters appear in group 1. If the text string is greater than 250 characters, the string is divided into 250-character chunks, which appear in one or more group 3 codes. If group 3 codes are used, the last group is a group 1 and has fewer than 250 characters.

3


Additional text (always in 250-character chunks) (optional)

7


DXF: X value; APP: 3D vectText style name (STANDARD if not provided) (optional)

210


Extrusion direction (optional; default = 0, 0, 1)

DXF: X value; APP: 3D vector

220,230


DXF: Y and Z values of extrusion direction (optional)

11


X-axis direction vector (in WCS)

DXF: X value; APP: 3D vector

21,31


DXF: Y and Z values of X-axis direction vector (in WCS)

42


Horizontal width of the characters that make up the mtext entity.

This value will always be equal to or less than the value of group code 41 (read-only, ignored if supplied).

43


Vertical height of the mtext entity (read-only, ignored if supplied)

50


Rotation angle in radians

73


Mtext line spacing style (optional):

1 = At least (taller characters will override)

2 = Exact (taller characters will not override)

44


Mtext line spacing factor (optional):

Percentage of default (3-on-5) line spacing to be applied.

Valid values range from 0.25 to 4.00

90


Background fill setting:

0 = Background fill off

1 = Use background fill color

2 = Use drawing window color as background fill color

63


Background color (if color index number)

420-429


Background color (if RGB color)

430-439


Background color (if color name)

45


Fill box scale (optional):

Determines how much border is around the text.

63


Background fill color (optional):

Color to use for background fill when group code 90 is 1.

441
    Transparency of background fill color (not implemented)

If group 72 and/or 74 values are nonzero then the text insertion point values are ignored, and new values are calculated by AutoCAD based on the text alignment point and the length of the text string itself (after applying the text style). If the 72 and 74 values are zero or missing, then the text alignment point is ignored and recalculated based on the text insertion point and the length of the text string itself (after applying the text style).


    */
};

} // namespace Dxf
