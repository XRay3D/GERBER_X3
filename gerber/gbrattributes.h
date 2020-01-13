#pragma once

#include <QMetaEnum>
#include <QMetaObject>
#include <QObject>
namespace Gerber {
namespace Att { //Attributes
    struct Command {
        static int value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }
        enum e {
            TF, // Attribute file. Set a file attribute.    5.2
            TA, // Attribute aperture. Add an aperture attribute to the dictionary or modify it.    5.3
            TO, // Attribute object. Add an object attribute to the dictionary or modify it.    5.4
            TD, // Attribute delete. Delete one or all attributes in the dictionary.    5.5
        };
        Q_ENUM(e)
        Q_GADGET
    };

    struct File {
        static int value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }
        enum e {
            Part, /*
        Identifies the part the file represents, e.g. a single PCB 5.6.2*/
            FileFunction, /*
        Identifies the file’s function in the PCB, e.g. top copper layer 5.6.3*/
            FilePolarity, /*
        Defines whether the file represents the presence or absence of material
        in the PCB layer, expressed by positive or negative 5.6.4*/
            SameCoordinates, /*
        All files in a fabrication data set with this attribute use the same
        coordinates. In other words, they align. 5.6.5*/
            CreationDate, /*
        Defines the creation date and time of the file. 5.6.6*/
            GenerationSoftware, /*
        Identifies the software creating the file. 5.6.7*/
            ProjectId, /*
        Defines project and revisions. 5.6.8*/
            MD5, /*
        Sets the MD5 file signature or checksum. 5.6.9*/
        };
        Q_ENUM(e)
        Q_GADGET
    };

    struct FileFunction {
        static int value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }
        enum e {
            // Data files
            Copper, /*L<p>,(Top|Inr|Bot)[,<type>]
        A conductor or copper layer.
        L<p> (p is an integer>0) specifies the physical copper layer number. Numbers are consecutive. The top layer is always L1. (L0 does not exist.) The mandatory field (Top|Inr|Bot) specifies it as the top, an inner or the bottom layer; this redundant information helps in handling partial data. The specification of the top layer is “Copper,L1,Top[,type]”, of the bottom layer of an 8 layer job it is Copper,L8,Bot[,type]
        The top side is the one with the through-hole components, if any.
        The optional <type> field indicates the layer type. If present it must take one of the following values: Plane, Signal, Mixed or Hatched. */
            Plated, /*i,j,(PTH|Blind|Buried) [,<label>]
        Plated drill/rout data, span from copper layer i to layer j. The from/to order is not significant. The (PTH|Blind|Buried) field is mandatory.
        The label is optional. If present it must take one of the following values: Drill, Rout or Mixed. */
            NonPlated, /*i,j,(NPTH|Blind|Buried) [,<label>]
        Non-plated drill/rout data, span from copper layer i to layer j. The from/to order is not significant. The (NPTH|Blind|Buried) field is mandatory.
        The label is optional. If present it must take one of the following values: Drill, Rout or Mixed. */
            Profile, /*(P|NP)
        A file containing the board profile (or outline) and only the board profile. Such a file is mandatory in a PCB fabrication data set. See 5.8.1.
        The mandatory (P|NP) label indicates whether board is edge-plated or not. */
            Soldermask, /*(Top|Bot)[,<index>]
        Solder mask or solder resist.
        Usually the image represents the solder mask openings; it then has negative polarity, see 5.6.4.
        The optional field is only needed when there is more than one solder mask on one side – top or bottom. The integer <index> then numbers the solder masks from the PCB side outwards, starting with 1 for the mask directly on the copper. Usually there is only one solder mask on a side, and then <index> is omitted. An example with two top solder masks:
        Soldermask,Top,1 Mask on the copper
        Soldermask,Top,2  Mask on the first mask */
            Legend, /*(Top|Bot)[,<index>]
        A legend is printed on top of the solder mask to show which component goes where. A.k.a. ‘silk’ or ‘silkscreen’.
        See the Soldermask entry for an explanation of the index. */
            Paste, /*(Top|Bot)
        Locations where paste must be applied. */
            Glue, /*(Top|Bot)
        Glue spots used to fix components to the board prior to soldering. */
            Carbonmask, /*(Top|Bot)[,<index>]
        See Soldermask for the usage of <index>. */
            Goldmask, /*(Top|Bot)[,<index>]
        See Soldermask for the usage of <index>. */
            Heatsinkmask,
            /*(Top|Bot)[,<index>]
        See Soldermask for the usage of <index>. */
            Peelablemask, /*(Top|Bot)[,<index>]
        See Soldermask for the usage of <index>. */
            Silvermask, /*(Top|Bot)[,<index>]
        See Soldermask for the usage of <index>. */
            Tinmask, /*(Top|Bot)[,<index>]
        See Soldermask for the usage of <index>. */
            Depthrout, /*(Top|Bot)
        Area that must be routed to a given depth rather than going through the whole board. */
            Vcut, /*[,(Top|Bot)]
        Contains the lines that must be v-cut. (V-cutting is also called scoring.)
        If the optional attachment (Top|Bot) is not present the scoring lines are identical on top and bottom – this is the normal case. In the exceptional case scoring is different on top and bottom two files must be supplied, one with Top and the other with Bot. */
            Viafill, /*
        Contains the via’s that must be filled. It is however recommended to specify the filled via’s with the optional field in the .AperFunction ViaDrill. */

            // Drawing files
            ArrayDrawing, /*
        A drawing of the array (biscuit, assembly panel, shipment panel, customer panel). */
            AssemblyDrawing, /*(Top|Bot)
        A drawing with the locations and reference designators of the components. It is mainly used in PCB assembly. */
            Drillmap, /*
        A drawing with the locations of the drilled holes. It often also contains the hole sizes, tolerances and plated/non-plated info. */
            FabricationDrawing, /*
        A drawing with additional information for the fabrication of the bare PCB: the location of holes and slots, the board outline, sizes and tolerances, layer stack, material, finish choice, etc. */
            Vcutmap, /*
        A drawing with v-cut or scoring information. */
            OtherDrawing, /*<mandatory field>
        Any other drawing than the 4 ones above. The mandatory field informally describes its topic. */
            //  Other files
            Pads, /*(Top|Bot)
        A file containing only the pads (SMD, BGA, component, …). Not needed in a fabrication data set. */
            Other, /*<mandatory field>
        The value ‘Other’ is to be used if none of the values above fits. By putting ‘Other’ rather than simply omitting the file function attribute it is clear the file has none of the standard functions, already useful information. Do not abuse standard values for a file with a vaguely similar function – use ‘Other’ to keep the function value clean and reliable.
        The mandatory field informally describes the file function.*/
            Component,
            /*L<p>,(Top|Bot) A component layer.
        L<p> (p is the copper layer number to which
        the components described in this file are attached) specifies the physical copper layer
        number. (Top|Bot) indicates if the components are on top, upwards, or on the bottom,
        downward, of the layer to which they are attached. This syntax caters for embedded
        components.
        For jobs without embedded components there is an intentional redundancy. This syntax
        caters for jobs with embedded components.
        Example:
        %TF.FileFunction,Component,L1,Top*%
        %TF.FileFunction,Component,L4,Bot*%*/
        };
        Q_ENUM(e)
        Q_GADGET
    };

    struct Aperture {
        static int value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }
        enum e {
            AperFunction, /*
        Function objects created with the apertures, e.g. SMD pad 5.6.10*/
            DrillTolerance, /*
        Tolerance of drill holes 5.6.11*/
            FlashText, /*
        If a flash represents text allows to define string, font, … 5.6.12*/
        };
        Q_ENUM(e)
        Q_GADGET
    };

    struct AperFunction {
        static int value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }
        enum e {
            ComponentMain, /* This aperture is flashed at the centroid of a component.
        The flash carries the object attributes with the main
        characteristics of the component.
        The following aperture must be used:
        %ADD10C,0.300*% (mm)
        %ADD10C,0.012*% (in)*/
            ComponentOutline, /*(Body|Lead2Lead|Footprint|Courtyard)
        This attribute is used to draw the outline of the
        component. An outline is a sequence of connected
        draws and arcs. They are said to connect only if they are
        defined consecutively, with the second starting where
        the first one ends. Thus, the order in which they are
        defined is significant. A contour is closed: the end point
        of the last draw/arc must coincide with the start point of
        the first. Outlines cannot self-intersect.
        Four different types of outlines are defined. See drawing,
        courtesy Thiadmer Riemersma:
        Outlines of different types on the same component are
        allowed.
        The following aperture must be used:
        %ADD11C,0.100*% (mm)
        %ADD11C,0.004*% (in)*/
            ComponentPin,
            /*An aperture whose flash point indicates the location of
        the component pins (leads). The .P object attribute must
        be attached to each flash to identify the reference
        descriptor and pin.
        For the key pin, typically pin "1" or "A1", the following
        diamond shape aperture must be used:
        %ADD12P,0.360X4X0.0*% (mm)
        %ADD12P,0.017X4X0.0*% (in)
        The key pin is then visible in the image.
        For all other pins the following zero size aperture must
        be used:
        %ADD13C,0*%...(both mm and in)
        These pins are not visible which avoids cluttering the
        image.*/
        };
        Q_ENUM(e)
        Q_GADGET
    };
    struct ComponentOutline {
        static int value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }
        enum e {
            Body,
            Lead2Lead,
            Footprint,
            Courtyard,
        };
        Q_ENUM(e)
        Q_GADGET
    };
}
}
