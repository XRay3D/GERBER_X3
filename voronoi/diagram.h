#ifndef DIAGRAM_H
#define DIAGRAM_H

#include "cell.h"
#include <QObject>
namespace Vrn {
class Diagram {
public:
    // ---------------------------------------------------------------------------
    // Diagram methods
    Diagram();
    ~Diagram();

    QVector<Cell*> cells;
    QVector<Edge*> edges;
    QVector<Vertex*> vertices;
    int execTime = 0;
};
}
#endif // DIAGRAM_H
