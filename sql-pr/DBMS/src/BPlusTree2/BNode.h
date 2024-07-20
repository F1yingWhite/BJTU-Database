//
// Created by BlackCat on 2023/3/23.
//
/*
 详细B+树数据请看
 https://zhuanlan.zhihu.com/p/149287061
 */

#ifndef BPLUSTREE_BNode_H
#define BPLUSTREE_BNode_H

#include "../Catalog/Struct/Attribute.h"
#include "../Catalog/Struct/varialbeType.h"
#include "vector"
#include "iostream"

using namespace std;

class BPlusTree;

enum Node_Type
{
    LEAF,
    INTERNAL
}; // 节点的数据类型,这样我们不需要记录树的高度

enum Sibling
{ // 用来记录兄弟节点是否充足
    LEFT,
    RIGHT,
    NONEHAVE
};

class BNode
{       // 一个BNode就是一个block
public: // 我们使用vector来存放数据
    BNode(int o, BNode *parent, string file) : order(o), fileName(file)
    {
        this->parent = parent;
        if (parent != nullptr)
        {
            parentOffset = parent->offset;
        }
        type = LEAF;
        changed = false;
        leftSibling = rightSibling = nullptr;
        if (parent != nullptr)
            parentOffset = parent->offset;
        else
            parentOffset = -1;
        leftOffset = rightOffset = -1; // 这个三个offset表示的是文件的偏移量
    }

    BNode(int o, int par, string file) : order(o), fileName(file)
    {
        parent = nullptr;
        this->parentOffset = par;
        type = LEAF;
        changed = false;
        leftSibling = rightSibling = nullptr;
        parentOffset = par;
        leftOffset = rightOffset = -1;
    }

    void setTree(BPlusTree *tree);

    int findKey(variableType key);   // 用来查找等于key的位置
    int findIndex(variableType key); // 用来查找第一个大于等于key的位置
    Node_Type getType() { return type; }

    // 二级索引
    BNode *createParent();

    void setIndex(int index, variableType key) { Index[index] = key; }

    int getNum() { return Index.size(); }                           // 获取节点内的元素个数
    variableType getIndex(int index) { return Index[index]; }       // 获取第i个元素
    variableType getLastIndex() { return Index[Index.size() - 1]; } // 获取最后一个元素的大小
    BNode *getParent() { return parent; }                           // 获取当且节点的父亲节点
    void setParent(BNode *par)
    {
        parent = par;
        if (par == nullptr)
            parentOffset = -1;
        else
            parentOffset = par->offset;
    }

    BNode *getLeftSibling();

    BNode *getRightSibling();

    virtual void split() = 0; // 自我分裂
    Sibling ifSiblingEnough();

public:
    int offset;                                // 自己的偏移量
    BPlusTree *tree;                           // 管理着该节所存在的树
    string fileName;                           // 用来存放当前节点的文件名
    bool changed;                              // 用来记录当前节点是否被修改过,如果修改过,则需要将其写回磁盘
    int order;                                 // 用来存放B+树的阶,虽然我觉得这样做有点占内存?不过这点开销可以接受
    vector<variableType> Index;                // 用来存放当前节点的索引数据
    BNode *parent;                             // 存放当前节点的父亲节点
    int parentOffset, leftOffset, rightOffset; // 存放的是文件的偏移量
    Node_Type type;                            // 记录当前节点的类型,是叶子还是卫星

    BNode *leftSibling, *rightSibling;
};

class IntervalNode : public BNode
{
public: // 删除了直接的内存指针改为了偏移量,
    IntervalNode(int o, BNode *parent, string file) : BNode(o, parent, file) { this->type = INTERNAL; }

    IntervalNode(int o, int par, string file) : BNode(o, par, file) { this->type = INTERNAL; }

    ~IntervalNode(); // 析构函数,用于写回磁盘

    int getLastPointerOffset() { return PointersOffset[PointersOffset.size() - 1]; }

    void insertKey(BNode *child); // 插入数据,我的想法是每次先插到最底部,然后再从底部向上返回
    void removeKey(BNode *child);

    void split(); // 自我分裂
    void updateIndex(variableType newKey, variableType oldKey);

    void combineWithSibling(Sibling where);

    void DeleteSelf();

public:
    vector<int> PointersOffset; // 记录下对应指针的偏移量,文件是随机存储随机访问的,内部节点直接记下偏移量
};

class LeafNode : public BNode
{
public:
    LeafNode(int o, BNode *parent, string file) : BNode(o, parent, file)
    {
        this->type = LEAF;
    }

    LeafNode(int o, int par, string file) : BNode(o, par, file)
    {
        this->type = LEAF;
    }

    ~LeafNode(); // 析构函数,用于写回磁盘
    int getData(int index) { return Data[index]; }

    void combineWithSibling(Sibling where);

    bool insertKey(variableType index, int data);

    int removeKey(variableType index);

    void split(); // 自我分裂

public:
    vector<int> Data; // 用来存放数据
};

#endif // BPLUSTREE_BNode_H
