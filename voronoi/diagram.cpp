#include "diagram.h"
namespace Vrn {
Diagram::Diagram() {}

Diagram::~Diagram()
{
    qDeleteAll(cells);
    qDeleteAll(edges);
    qDeleteAll(vertices);
}
}
