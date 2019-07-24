#include "rbtree.h"
namespace Vrn {
int cNode = 0;

Node::Node()
{
    //qDebug() << "Node" << ++cNode;
}

Node::~Node()
{
    //qDebug() << "Node" << cNode--;
}

RBTree::RBTree()
{
    //this->root = new Node; // nullptr;
}

void RBTree::rbInsertSuccessor(Node* node, Node* successor)
{
    Node* parent = nullptr;
    if (node) {
        // >>> rhill 2011-05-27: Performance: cache previous/next nodes
        successor->rbPrevious = node;
        successor->rbNext = node->rbNext;
        if (node->rbNext) {
            node->rbNext->rbPrevious = successor;
        }
        node->rbNext = successor;
        // <<<
        if (node->rbRight) {
            // in-place expansion of node.rbRight.getFirst();
            node = node->rbRight;
            while (node->rbLeft) {
                node = node->rbLeft;
            }
            node->rbLeft = successor;
        } else {
            node->rbRight = successor;
        }
        parent = node;
    }
    // rhill 2011-06-07: if node is null, successor must be inserted
    // to the left-most part of the tree
    else if (this->root) {
        node = getFirst(this->root);
        // >>> Performance: cache previous/next nodes
        successor->rbPrevious = nullptr;
        successor->rbNext = node;
        node->rbPrevious = successor;
        // <<<
        node->rbLeft = successor;
        parent = node;
    } else {
        // >>> Performance: cache previous/next nodes
        //delete successor->rbPrevious;
        //delete successor->rbNext;
        successor->rbPrevious = successor->rbNext = nullptr;

        // <<<
        this->root = successor;
        //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter converted the original 'null' assignment to a call to 'delete', but you should review memory allocation of all pointer variables in the converted code:
        /*delete*/ parent = nullptr;
    }
    //delete successor->rbLeft;
    //delete successor->rbRight;
    successor->rbLeft = successor->rbRight = nullptr;
    successor->rbParent = parent;
    successor->rbRed = true;
    // Fixup the modified tree by recoloring nodes and performing
    // rotations (2 at most) hence the red-black tree properties are
    // preserved.
    Node *grandpa = nullptr, *uncle = nullptr;
    node = successor;
    while (parent && parent->rbRed) {
        grandpa = parent->rbParent;
        if (parent == grandpa->rbLeft) {
            uncle = grandpa->rbRight;
            if (uncle && uncle->rbRed) {
                parent->rbRed = uncle->rbRed = false;
                grandpa->rbRed = true;
                node = grandpa;
            } else {
                if (node == parent->rbRight) {
                    rbRotateLeft(parent);
                    node = parent;
                    parent = node->rbParent;
                }
                parent->rbRed = false;
                grandpa->rbRed = true;
                rbRotateRight(grandpa);
            }
        } else {
            uncle = grandpa->rbLeft;
            if (uncle && uncle->rbRed) {
                parent->rbRed = uncle->rbRed = false;
                grandpa->rbRed = true;
                node = grandpa;
            } else {
                if (node == parent->rbLeft) {
                    rbRotateRight(parent);
                    node = parent;
                    parent = node->rbParent;
                }
                parent->rbRed = false;
                grandpa->rbRed = true;
                rbRotateLeft(grandpa);
            }
        }
        parent = node->rbParent;
    }
    this->root->rbRed = false;
}

void RBTree::rbRemoveNode(Node* node)
{
    // >>> rhill 2011-05-27: Performance: cache previous/next nodes
    if (node->rbNext) {
        node->rbNext->rbPrevious = node->rbPrevious;
    }
    if (node->rbPrevious) {
        node->rbPrevious->rbNext = node->rbNext;
    }
    //delete node->rbNext;
    //delete node->rbPrevious;
    node->rbNext = node->rbPrevious = nullptr;
    // <<<
    Node *parent = node->rbParent, *left = node->rbLeft, *right = node->rbRight, *next;
    if (!left) {
        next = right;
    } else if (!right) {
        next = left;
    } else {
        next = getFirst(right);
    }
    if (parent) {
        if (parent->rbLeft == node) {
            parent->rbLeft = next;
        } else {
            parent->rbRight = next;
        }
    } else {
        this->root = next;
    }
    // enforce red-black rules
    bool isRed;
    if (left && right) {
        isRed = next->rbRed;
        next->rbRed = node->rbRed;
        next->rbLeft = left;
        left->rbParent = next;
        if (next != right) {
            parent = next->rbParent;
            next->rbParent = node->rbParent;
            node = next->rbRight;
            parent->rbLeft = node;
            next->rbRight = right;
            right->rbParent = next;
        } else {
            next->rbParent = parent;
            parent = next;
            node = next->rbRight;
        }
    } else {
        isRed = node->rbRed;
        node = next;
    }
    // 'node' is now the sole successor's child and 'parent' its
    // new parent (since the successor can have been moved)
    if (node) {
        node->rbParent = parent;
    }
    // the 'easy' cases
    if (isRed) {
        return;
    }
    if (node && node->rbRed) {
        node->rbRed = false;
        return;
    }
    // the other cases
    Node* sibling;
    do {
        if (node == this->root) {
            break;
        }
        if (node == parent->rbLeft) {
            sibling = parent->rbRight;
            if (sibling->rbRed) {
                sibling->rbRed = false;
                parent->rbRed = true;
                rbRotateLeft(parent);
                sibling = parent->rbRight;
            }
            if ((sibling->rbLeft && sibling->rbLeft->rbRed) || (sibling->rbRight && sibling->rbRight->rbRed)) {
                if (!sibling->rbRight || !sibling->rbRight->rbRed) {
                    sibling->rbLeft->rbRed = false;
                    sibling->rbRed = true;
                    rbRotateRight(sibling);
                    sibling = parent->rbRight;
                }
                sibling->rbRed = parent->rbRed;
                parent->rbRed = sibling->rbRight->rbRed = false;
                rbRotateLeft(parent);
                node = this->root;
                break;
            }
        } else {
            sibling = parent->rbLeft;
            if (sibling->rbRed) {
                sibling->rbRed = false;
                parent->rbRed = true;
                rbRotateRight(parent);
                sibling = parent->rbLeft;
            }
            if ((sibling->rbLeft && sibling->rbLeft->rbRed) || (sibling->rbRight && sibling->rbRight->rbRed)) {
                if (!sibling->rbLeft || !sibling->rbLeft->rbRed) {
                    sibling->rbRight->rbRed = false;
                    sibling->rbRed = true;
                    rbRotateLeft(sibling);
                    sibling = parent->rbLeft;
                }
                sibling->rbRed = parent->rbRed;
                parent->rbRed = sibling->rbLeft->rbRed = false;
                rbRotateRight(parent);
                node = this->root;
                break;
            }
        }
        sibling->rbRed = true;
        node = parent;
        parent = parent->rbParent;
    } while (!node->rbRed);
    if (node) {
        node->rbRed = false;
    }
}

void RBTree::rbRotateLeft(Node* node)
{
    Node *p = node, *q = node->rbRight, *parent = p->rbParent; // can't be null
    if (parent) {
        if (parent->rbLeft == p) {
            parent->rbLeft = q;
        } else {
            parent->rbRight = q;
        }
    } else {
        this->root = q;
    }
    q->rbParent = parent;
    p->rbParent = q;
    p->rbRight = q->rbLeft;
    if (p->rbRight) {
        p->rbRight->rbParent = p;
    }
    q->rbLeft = p;
}

void RBTree::rbRotateRight(Node* node)
{
    Node *p = node, *q = node->rbLeft, *parent = p->rbParent; // can't be null
    if (parent) {
        if (parent->rbLeft == p) {
            parent->rbLeft = q;
        } else {
            parent->rbRight = q;
        }
    } else {
        this->root = q;
    }
    q->rbParent = parent;
    p->rbParent = q;
    p->rbLeft = q->rbRight;
    if (p->rbLeft) {
        p->rbLeft->rbParent = p;
    }
    q->rbRight = p;
}

Node* RBTree::getFirst(Node* node)
{
    while (node->rbLeft) {
        node = node->rbLeft;
    }
    return node;
}

Node* RBTree::getLast(Node* node)
{
    while (node->rbRight) {
        node = node->rbRight;
    }
    return node;
}
}
