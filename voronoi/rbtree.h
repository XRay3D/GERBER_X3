#ifndef RBTREE_H
#define RBTREE_H

#include "cell.h"
#include "circleevent.h"
#include <QObject>
namespace Vrn {
//#define CircleEvent Node

// ---------------------------------------------------------------------------
// Red-Black tree code (based on C version of "rbtree" by Franck Bui-Huu
// https://github.com/fbuihuu/libtree/blob/master/rb.c

//class CircleEvent : public Node {
//public:
//    CircleEvent() {}
//    Node* arc = nullptr;
//    //    Node* rbLeft = nullptr;
//    //    Node* rbNext = nullptr;
//    //    Node* rbParent = nullptr;
//    //    Node* rbPrevious = nullptr;
//    //    bool rbRed = false;
//    //    Node* rbRight = nullptr;
//    //    Vertex* site;
//    double x = 0.0;
//    double y = 0.0;
//    double ycenter = 0.0;
//};

class RBTree {
public:
    RBTree();

    void rbInsertSuccessor(Node* node, Node* successor);

    void rbRemoveNode(Node* node);

    void rbRotateLeft(Node* node);

    void rbRotateRight(Node* node);

    Node* getFirst(Node* node);

    Node* getLast(Node* node);

    Node* root = nullptr;
};
}
#endif // RBTREE_H
