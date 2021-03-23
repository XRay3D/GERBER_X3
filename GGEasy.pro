#-------------------------------------------------
#
# Project created by QtCreator 2014-05-23T17:51:21
#
#-------------------------------------------------

QT = core gui opengl serialport script uitools widgets



TRANSLATIONS += \
    GGEasy\translations/GGEasy_en.ts \
    GGEasy\translations/GGEasy_es.ts \
    GGEasy\translations/GGEasy_fr.ts \
    GGEasy\translations/GGEasy_pt.ts \
    GGEasy\translations/GGEasy_ru.ts \

SOURCES += \
    GGEasy\aboutform.cpp \
    GGEasy\forms\drillform\drillform.cpp \
    GGEasy\forms\drillform\drillmodel.cpp \
    GGEasy\forms\formsutil\errordialog.cpp \
    GGEasy\forms\formsutil\formsutil.cpp \
    GGEasy\forms\gcodepropertiesform.cpp \
    GGEasy\forms\hatchingform.cpp \
    GGEasy\forms\pocketoffsetform.cpp \
    GGEasy\forms\pocketrasterform.cpp \
    GGEasy\forms\profileform.cpp \
    GGEasy\forms\toolname.cpp \
    GGEasy\forms\voronoiform.cpp \
    GGEasy\gcode\gccreator.cpp \
    GGEasy\gcode\gcdrillitem.cpp \
    GGEasy\gcode\gcfile.cpp \
    GGEasy\gcode\gch.cpp \
    GGEasy\gcode\gchatching.cpp \
    GGEasy\gcode\gcnode.cpp \
    GGEasy\gcode\gcplugin.cpp \
    GGEasy\gcode\gcpocketoffset.cpp \
    GGEasy\gcode\gcpocketraster.cpp \
    GGEasy\gcode\gcprofile.cpp \
    GGEasy\gcode\gcthermal.cpp \
    GGEasy\gcode\gcutils.cpp \
    GGEasy\gcode\voroni\gcvoronoi.cpp \
    GGEasy\gcode\voroni\gcvoronoiboost.cpp \
    GGEasy\gcode\voroni\gcvoronoicgal.cpp \
    GGEasy\gcode\voroni\gcvoronoijc.cpp \
    GGEasy\gcode\voroni\jc_voronoi.cpp \
    GGEasy\main.cpp \
    GGEasy\mainwindow.cpp \
    GGEasy\plugindialog.cpp \
    GGEasy\point.cpp \
    GGEasy\recent.cpp \
    GGEasy\settingsdialog.cpp \
    GGEasy\style.cpp \
    file_plugins\dxf\dxf_block.cpp \
    file_plugins\dxf\dxf_codedata.cpp \
    file_plugins\dxf\dxf_file.cpp \
    file_plugins\dxf\dxf_node.cpp \
    file_plugins\dxf\dxf_plugin.cpp \
    file_plugins\dxf\dxf_settingstab.cpp \
    file_plugins\dxf\dxf_sourcedialog.cpp \
    file_plugins\dxf\dxf_types.cpp \
    file_plugins\dxf\entities\dxf_arc.cpp \
    file_plugins\dxf\entities\dxf_attdef.cpp \
    file_plugins\dxf\entities\dxf_attrib.cpp \
    file_plugins\dxf\entities\dxf_body.cpp \
    file_plugins\dxf\entities\dxf_circle.cpp \
    file_plugins\dxf\entities\dxf_dimension.cpp \
    file_plugins\dxf\entities\dxf_dummy.cpp \
    file_plugins\dxf\entities\dxf_ellipse.cpp \
    file_plugins\dxf\entities\dxf_entity.cpp \
    file_plugins\dxf\entities\dxf_graphicobject.cpp \
    file_plugins\dxf\entities\dxf_hatch.cpp \
    file_plugins\dxf\entities\dxf_helix.cpp \
    file_plugins\dxf\entities\dxf_image.cpp \
    file_plugins\dxf\entities\dxf_insert.cpp \
    file_plugins\dxf\entities\dxf_leader.cpp \
    file_plugins\dxf\entities\dxf_light.cpp \
    file_plugins\dxf\entities\dxf_line.cpp \
    file_plugins\dxf\entities\dxf_lwpolyline.cpp \
    file_plugins\dxf\entities\dxf_mesh.cpp \
    file_plugins\dxf\entities\dxf_mleader.cpp \
    file_plugins\dxf\entities\dxf_mleaderstyle.cpp \
    file_plugins\dxf\entities\dxf_mline.cpp \
    file_plugins\dxf\entities\dxf_mtext.cpp \
    file_plugins\dxf\entities\dxf_ole2frame.cpp \
    file_plugins\dxf\entities\dxf_oleframe.cpp \
    file_plugins\dxf\entities\dxf_point.cpp \
    file_plugins\dxf\entities\dxf_polyline.cpp \
    file_plugins\dxf\entities\dxf_ray.cpp \
    file_plugins\dxf\entities\dxf_region.cpp \
    file_plugins\dxf\entities\dxf_section.cpp \
    file_plugins\dxf\entities\dxf_shape.cpp \
    file_plugins\dxf\entities\dxf_solid.cpp \
    file_plugins\dxf\entities\dxf_spline.cpp \
    file_plugins\dxf\entities\dxf_sun.cpp \
    file_plugins\dxf\entities\dxf_surface.cpp \
    file_plugins\dxf\entities\dxf_table.cpp \
    file_plugins\dxf\entities\dxf_text.cpp \
    file_plugins\dxf\entities\dxf_tolerance.cpp \
    file_plugins\dxf\entities\dxf_trace.cpp \
    file_plugins\dxf\entities\dxf_underlay.cpp \
    file_plugins\dxf\entities\dxf_vec2.cpp \
    file_plugins\dxf\entities\dxf_vertex.cpp \
    file_plugins\dxf\entities\dxf_viewport.cpp \
    file_plugins\dxf\entities\dxf_wipeout.cpp \
    file_plugins\dxf\entities\dxf_xline.cpp \
    file_plugins\dxf\section\dxf_blocks.cpp \
    file_plugins\dxf\section\dxf_classes.cpp \
    file_plugins\dxf\section\dxf_entities.cpp \
    file_plugins\dxf\section\dxf_headerparser.cpp \
    file_plugins\dxf\section\dxf_objects.cpp \
    file_plugins\dxf\section\dxf_sectionparser.cpp \
    file_plugins\dxf\section\dxf_tables.cpp \
    file_plugins\dxf\section\dxf_thumbnailimage.cpp \
    file_plugins\dxf\tables\dxf_abstracttable.cpp \
    file_plugins\dxf\tables\dxf_appid.cpp \
    file_plugins\dxf\tables\dxf_block_record.cpp \
    file_plugins\dxf\tables\dxf_dimstyle.cpp \
    file_plugins\dxf\tables\dxf_layer.cpp \
    file_plugins\dxf\tables\dxf_layermodel.cpp \
    file_plugins\dxf\tables\dxf_ltype.cpp \
    file_plugins\dxf\tables\dxf_style.cpp \
    file_plugins\dxf\tables\dxf_ucs.cpp \
    file_plugins\dxf\tables\dxf_view.cpp \
    file_plugins\dxf\tables\dxf_vport.cpp \
    file_plugins\excellon\drillitem.cpp \
    file_plugins\excellon\excellondialog.cpp \
    file_plugins\excellon\exfile.cpp \
    file_plugins\excellon\exformatstate.cpp \
    file_plugins\excellon\exh.cpp \
    file_plugins\excellon\exnode.cpp \
    file_plugins\excellon\exparser.cpp \
    file_plugins\excellon\explugin.cpp \
    file_plugins\gerber\compdialog.cpp \
    file_plugins\gerber\compitem.cpp \
    file_plugins\gerber\compmodel.cpp \
    file_plugins\gerber\compnode.cpp \
    file_plugins\gerber\compview.cpp \
    file_plugins\gerber\gbraperture.cpp \
    file_plugins\gerber\gbrattraperfunction.cpp \
    file_plugins\gerber\gbrattrfilefunction.cpp \
    file_plugins\gerber\gbrattributes.cpp \
    file_plugins\gerber\gbrcomponent.cpp \
    file_plugins\gerber\gbrfile.cpp \
    file_plugins\gerber\gbrh.cpp \
    file_plugins\gerber\gbrnode.cpp \
    file_plugins\gerber\gbrparser.cpp \
    file_plugins\gerber\gbrplugin.cpp \
    file_plugins\gerber\mathparser.cpp \
    file_plugins\hpgl\hpgl_file.cpp \
    file_plugins\hpgl\hpgl_node.cpp \
    file_plugins\hpgl\hpgl_parser.cpp \
    file_plugins\hpgl\hpgl_plugin.cpp \
    file_plugins\hpgl\hpgl_sourcedialog.cpp \
    file_plugins\hpgl\hpgl_types.cpp \
    shape_plugins\circle\shcircle.cpp \
    shape_plugins\circlearc\sharc.cpp \
    shape_plugins\polyline\shpolyline.cpp \
    shape_plugins\rectangle\shrectangle.cpp \
    shape_plugins\shape\shape.cpp \
    shape_plugins\shape\shhandler.cpp \
    shape_plugins\shape\shnode.cpp \
    shape_plugins\text\shtext.cpp \
    shape_plugins\text\shtextdialog.cpp \
    static_libs\clipper\clipper.cpp \
    static_libs\clipper\cpline.cpp \
    static_libs\clipper\myclipper.cpp \
    static_libs\common\colorselector.cpp \
    static_libs\common\depthform.cpp \
    static_libs\common\doublespinbox.cpp \
    static_libs\common\project.cpp \
    static_libs\common\settings.cpp \
    static_libs\filetree\ft_foldernode.cpp \
    static_libs\filetree\ft_model.cpp \
    static_libs\filetree\ft_node.cpp \
    static_libs\filetree\ft_radiodelegate.cpp \
    static_libs\filetree\ft_sidedelegate.cpp \
    static_libs\filetree\ft_textdelegate.cpp \
    static_libs\filetree\ft_typedelegate.cpp \
    static_libs\filetree\ft_view.cpp \
    static_libs\gi\bridgeitem.cpp \
    static_libs\gi\datapathitem.cpp \
    static_libs\gi\datasoliditem.cpp \
    static_libs\gi\drillitem.cpp \
    static_libs\gi\drillpreviewgi.cpp \
    static_libs\gi\erroritem.cpp \
    static_libs\gi\gcpathitem.cpp \
    static_libs\gi\graphicsitem.cpp \
    static_libs\gi\itemgroup.cpp \
    static_libs\gi\thermalpreviewitem.cpp \
    static_libs\graphicsview\edid.cpp \
    static_libs\graphicsview\graphicsview.cpp \
    static_libs\graphicsview\qdruler.cpp \
    static_libs\graphicsview\scene.cpp \
    static_libs\thermal\thermaldelegate.cpp \
    static_libs\thermal\thermalform.cpp \
    static_libs\thermal\thermalmodel.cpp \
    static_libs\thermal\thermalnode.cpp \
    static_libs\tooldatabase\tool.cpp \
    static_libs\tooldatabase\tooldatabase.cpp \
    static_libs\tooldatabase\tooleditdialog.cpp \
    static_libs\tooldatabase\tooleditform.cpp \
    static_libs\tooldatabase\toolitem.cpp \
    static_libs\tooldatabase\toolmodel.cpp \
    static_libs\tooldatabase\toolselectorform.cpp \
    static_libs\tooldatabase\tooltreeview.cpp \


HEADERS  += \
    GGEasy\aboutform.h \
    GGEasy\forms\drillform\drillform.h \
    GGEasy\forms\drillform\drillmodel.h \
    GGEasy\forms\formsutil\errordialog.h \
    GGEasy\forms\formsutil\formsutil.h \
    GGEasy\forms\gcodepropertiesform.h \
    GGEasy\forms\hatchingform.h \
    GGEasy\forms\pocketoffsetform.h \
    GGEasy\forms\pocketrasterform.h \
    GGEasy\forms\profileform.h \
    GGEasy\forms\toolname.h \
    GGEasy\forms\voronoiform.h \
    GGEasy\gcode\gccreator.h \
    GGEasy\gcode\gcdrillitem.h \
    GGEasy\gcode\gcfile.h \
    GGEasy\gcode\gch.h \
    GGEasy\gcode\gchatching.h \
    GGEasy\gcode\gcnode.h \
    GGEasy\gcode\gcode.h \
    GGEasy\gcode\gcplugin.h \
    GGEasy\gcode\gcpocketoffset.h \
    GGEasy\gcode\gcpocketraster.h \
    GGEasy\gcode\gcprofile.h \
    GGEasy\gcode\gcthermal.h \
    GGEasy\gcode\gctypes.h \
    GGEasy\gcode\gcutils.h \
    GGEasy\gcode\voroni\gcvoronoi.h \
    GGEasy\gcode\voroni\gcvoronoiboost.h \
    GGEasy\gcode\voroni\gcvoronoicgal.h \
    GGEasy\gcode\voroni\gcvoronoijc.h \
    GGEasy\gcode\voroni\jc_voronoi.h \
    GGEasy\interfaces\file.h \
    GGEasy\interfaces\pluginfile.h \
    GGEasy\interfaces\plugintypes.h \
    GGEasy\interfaces\shapepluginin.h \
    GGEasy\mainwindow.h \
    GGEasy\messageoutput.h \
    GGEasy\openingdialog.h \
    GGEasy\plugindialog.h \
    GGEasy\point.h \
    GGEasy\recent.h \
    GGEasy\settingsdialog.h \
    GGEasy\splashscreen.h \
    GGEasy\style.h \
    GGEasy\version.h \
    file_plugins\dxf\dxf_block.h \
    file_plugins\dxf\dxf_codedata.h \
    file_plugins\dxf\dxf_file.h \
    file_plugins\dxf\dxf_node.h \
    file_plugins\dxf\dxf_plugin.h \
    file_plugins\dxf\dxf_settingstab.h \
    file_plugins\dxf\dxf_sourcedialog.h \
    file_plugins\dxf\dxf_types.h \
    file_plugins\dxf\entities\dxf_allentities.h \
    file_plugins\dxf\entities\dxf_arc.h \
    file_plugins\dxf\entities\dxf_attdef.h \
    file_plugins\dxf\entities\dxf_attrib.h \
    file_plugins\dxf\entities\dxf_body.h \
    file_plugins\dxf\entities\dxf_circle.h \
    file_plugins\dxf\entities\dxf_dimension.h \
    file_plugins\dxf\entities\dxf_dummy.h \
    file_plugins\dxf\entities\dxf_ellipse.h \
    file_plugins\dxf\entities\dxf_entity.h \
    file_plugins\dxf\entities\dxf_graphicobject.h \
    file_plugins\dxf\entities\dxf_hatch.h \
    file_plugins\dxf\entities\dxf_helix.h \
    file_plugins\dxf\entities\dxf_image.h \
    file_plugins\dxf\entities\dxf_insert.h \
    file_plugins\dxf\entities\dxf_leader.h \
    file_plugins\dxf\entities\dxf_light.h \
    file_plugins\dxf\entities\dxf_line.h \
    file_plugins\dxf\entities\dxf_lwpolyline.h \
    file_plugins\dxf\entities\dxf_mesh.h \
    file_plugins\dxf\entities\dxf_mleader.h \
    file_plugins\dxf\entities\dxf_mleaderstyle.h \
    file_plugins\dxf\entities\dxf_mline.h \
    file_plugins\dxf\entities\dxf_mtext.h \
    file_plugins\dxf\entities\dxf_ole2frame.h \
    file_plugins\dxf\entities\dxf_oleframe.h \
    file_plugins\dxf\entities\dxf_point.h \
    file_plugins\dxf\entities\dxf_polyline.h \
    file_plugins\dxf\entities\dxf_ray.h \
    file_plugins\dxf\entities\dxf_region.h \
    file_plugins\dxf\entities\dxf_section.h \
    file_plugins\dxf\entities\dxf_shape.h \
    file_plugins\dxf\entities\dxf_solid.h \
    file_plugins\dxf\entities\dxf_spline.h \
    file_plugins\dxf\entities\dxf_sun.h \
    file_plugins\dxf\entities\dxf_surface.h \
    file_plugins\dxf\entities\dxf_table.h \
    file_plugins\dxf\entities\dxf_text.h \
    file_plugins\dxf\entities\dxf_tolerance.h \
    file_plugins\dxf\entities\dxf_trace.h \
    file_plugins\dxf\entities\dxf_underlay.h \
    file_plugins\dxf\entities\dxf_vec2.h \
    file_plugins\dxf\entities\dxf_vertex.h \
    file_plugins\dxf\entities\dxf_viewport.h \
    file_plugins\dxf\entities\dxf_wipeout.h \
    file_plugins\dxf\entities\dxf_xline.h \
    file_plugins\dxf\section\dxf_blocks.h \
    file_plugins\dxf\section\dxf_classes.h \
    file_plugins\dxf\section\dxf_entities.h \
    file_plugins\dxf\section\dxf_headerparser.h \
    file_plugins\dxf\section\dxf_objects.h \
    file_plugins\dxf\section\dxf_sectionparser.h \
    file_plugins\dxf\section\dxf_tables.h \
    file_plugins\dxf\section\dxf_thumbnailimage.h \
    file_plugins\dxf\tables\dxf_abstracttable.h \
    file_plugins\dxf\tables\dxf_appid.h \
    file_plugins\dxf\tables\dxf_block_record.h \
    file_plugins\dxf\tables\dxf_dimstyle.h \
    file_plugins\dxf\tables\dxf_layer.h \
    file_plugins\dxf\tables\dxf_layermodel.h \
    file_plugins\dxf\tables\dxf_ltype.h \
    file_plugins\dxf\tables\dxf_style.h \
    file_plugins\dxf\tables\dxf_ucs.h \
    file_plugins\dxf\tables\dxf_view.h \
    file_plugins\dxf\tables\dxf_vport.h \
    file_plugins\excellon\drillitem.h \
    file_plugins\excellon\excellon.h \
    file_plugins\excellon\excellondialog.h \
    file_plugins\excellon\exfile.h \
    file_plugins\excellon\exh.h \
    file_plugins\excellon\exnode.h \
    file_plugins\excellon\exparser.h \
    file_plugins\excellon\explugin.h \
    file_plugins\excellon\extypes.h \
    file_plugins\gerber\compdialog.h \
    file_plugins\gerber\compitem.h \
    file_plugins\gerber\compmodel.h \
    file_plugins\gerber\compnode.h \
    file_plugins\gerber\compview.h \
    file_plugins\gerber\gbraperture.h \
    file_plugins\gerber\gbrattraperfunction.h \
    file_plugins\gerber\gbrattrfilefunction.h \
    file_plugins\gerber\gbrattributes.h \
    file_plugins\gerber\gbrcomponent.h \
    file_plugins\gerber\gbrfile.h \
    file_plugins\gerber\gbrh.h \
    file_plugins\gerber\gbrnode.h \
    file_plugins\gerber\gbrparser.h \
    file_plugins\gerber\gbrplugin.h \
    file_plugins\gerber\gbrtypes.h \
    file_plugins\gerber\mathparser.h \
    file_plugins\hpgl\hpgl_file.h \
    file_plugins\hpgl\hpgl_node.h \
    file_plugins\hpgl\hpgl_parser.h \
    file_plugins\hpgl\hpgl_plugin.h \
    file_plugins\hpgl\hpgl_sourcedialog.h \
    file_plugins\hpgl\hpgl_types.h \
    shape_plugins\circle\shcircle.h \
    shape_plugins\circlearc\sharc.h \
    shape_plugins\polyline\shpolyline.h \
    shape_plugins\rectangle\shrectangle.h \
    shape_plugins\shape\shape.h \
    shape_plugins\shape\shhandler.h \
    shape_plugins\shape\shheaders.h \
    shape_plugins\shape\shnode.h \
    shape_plugins\text\shtext.h \
    shape_plugins\text\shtextdialog.h \
    static_libs\clipper\cpline.h \
    static_libs\clipper\myclipper.h \
    static_libs\common\app.h \
    static_libs\common\colorselector.h \
    static_libs\common\datastream.h \
    static_libs\common\depthform.h \
    static_libs\common\doublespinbox.h \
    static_libs\common\leakdetector.h \
    static_libs\common\mvector.h \
    static_libs\common\project.h \
    static_libs\common\settings.h \
    static_libs\filetree\ft_foldernode.h \
    static_libs\filetree\ft_model.h \
    static_libs\filetree\ft_node.h \
    static_libs\filetree\ft_radiodelegate.h \
    static_libs\filetree\ft_sidedelegate.h \
    static_libs\filetree\ft_textdelegate.h \
    static_libs\filetree\ft_typedelegate.h \
    static_libs\filetree\ft_view.h \
    static_libs\gi\bridgeitem.h \
    static_libs\gi\datapathitem.h \
    static_libs\gi\datasoliditem.h \
    static_libs\gi\drillitem.h \
    static_libs\gi\drillpreviewgi.h \
    static_libs\gi\erroritem.h \
    static_libs\gi\gcpathitem.h \
    static_libs\gi\graphicsitem.h \
    static_libs\gi\itemgroup.h \
    static_libs\gi\thermalpreviewitem.h \
    static_libs\graphicsview\edid.h \
    static_libs\graphicsview\graphicsview.h \
    static_libs\graphicsview\qdruler.h \
    static_libs\graphicsview\scene.h \
    static_libs\thermal\thermal.h \
    static_libs\thermal\thermaldelegate.h \
    static_libs\thermal\thermalform.h \
    static_libs\thermal\thermalmodel.h \
    static_libs\thermal\thermalnode.h \
    static_libs\thermal\thvars.h \
    static_libs\tooldatabase\tool.h \
    static_libs\tooldatabase\tooldatabase.h \
    static_libs\tooldatabase\tooleditdialog.h \
    static_libs\tooldatabase\tooleditform.h \
    static_libs\tooldatabase\toolitem.h \
    static_libs\tooldatabase\toolmodel.h \
    static_libs\tooldatabase\toolselectorform.h \
    static_libs\tooldatabase\tooltreeview.h \


FORMS += \
    GGEasy\aboutform.ui \
    GGEasy\forms\drillform\drillform.ui \
    GGEasy\forms\gcodepropertiesform.ui \
    GGEasy\forms\hatchingform.ui \
    GGEasy\forms\pocketoffsetform.ui \
    GGEasy\forms\pocketrasterform.ui \
    GGEasy\forms\profileform.ui \
    GGEasy\forms\voronoiform.ui \
    GGEasy\mainwindow.ui \
    GGEasy\settingsdialog.ui \
    GTE_Win\mainwindow.ui \
    file_plugins\excellon\excellondialog.ui \
    shape_plugins\text\shtextdialog.ui \
    static_libs\thermal\thermalform.ui \
    static_libs\tooldatabase\tooldatabase.ui \
    static_libs\tooldatabase\tooleditdialog.ui \
    static_libs\tooldatabase\tooleditform.ui \



#qtPrepareTool(LRELEASE, lrelease)
#for(tsfile, TRANSLATIONS) {
#    qmfile = $$tsfile
#    qmfile ~= s,.ts$,.qm,
#    qmdir = $$dirname(qmfile)
#    !exists($$qmdir) {
#        mkpath($$qmdir)|error("Aborting.")
#    }
#    command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
#    system($$command)|error("Failed to run: $$command")
#}