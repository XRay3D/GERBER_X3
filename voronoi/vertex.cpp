#include "vertex.h"
namespace Vrn {
int cVertex = 0;

Vertex::Vertex(double x, double y)
    : x(x)
    , y(y)
{
}

Vertex::Vertex(double x, double y, int id)
    : x(x)
    , y(y)
    , id(id)
{
    //qDebug() << "Vertex" << ++cVertex;
}

Vertex::~Vertex()
{
    //qDebug() << "~Vertex" << cVertex--;
}
}
