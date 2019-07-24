#ifndef CIRCLEEVENT_H
#define CIRCLEEVENT_H

#include "rbtree.h"
namespace Vrn {
#define CircleEvent Node

class Node {
public:
    Node();
    ~Node();
    CircleEvent* circleEvent = nullptr;
    Edge* edge = nullptr;
    Node* rbLeft = nullptr;
    Node* rbNext = nullptr;
    Node* rbParent = nullptr;
    Node* rbPrevious = nullptr;
    bool rbRed = false;
    Node* rbRight = nullptr;
    Vertex* site = nullptr;
    //CircleEvent
    Node* arc = nullptr;
    double x = 0.0;
    double y = 0.0;
    double ycenter = 0.0;
};
}
#endif // CIRCLEEVENT_H
