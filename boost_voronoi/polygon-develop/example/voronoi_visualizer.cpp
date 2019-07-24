// Boost.Polygon library voronoi_visualizer.cpp file

//          Copyright Andrii Sydorchuk 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org for updates, documentation, and revision history.

#include <QApplication>

#include <iostream>
#include <vector>

#include <QtGui/QtGui>
#include <QtOpenGL/QGLWidget>

#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>
using namespace boost::polygon;

#include "mainwindow.h"
#include "voronoi_visual_utils.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

//#include "voronoi_visualizer.moc"
