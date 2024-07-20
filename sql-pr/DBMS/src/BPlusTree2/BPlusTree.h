//
// Created by BlackCat on 2023/3/23.
//

#ifndef BPLUSTREE_BPLUSTREE_H
#define BPLUSTREE_BPLUSTREE_H
#include "BNode.h"
#include "stack"
#include "queue"
#include "../Catalog/Struct/Attribute.h"
#include "../Catalog/Struct/varialbeType.h"
#include "fstream"
#include "list"
#include "unordered_map"
#include "string"
#include "CacheHead.h"

using namespace std;

void createIndexFile(vector<Attribute> &types, string tableName);

void setRoot(string fileName, int offset);

void setMaxLeft(string fileName, int offset);

BNode *getNode(string fileName, int offset);

BNode *getRoot(string fileName);

BNode *getMaxLeft(string fileName);

vector<Attribute> getIndexType(string fileName);

void deleteNode(string fileName, int offset);

void updateNode(string fileName, BNode *node);

int getNewNodeOffset(string fileName);

// 我们只用来减少读取次数,写回次数不变,也就是说我们每次想要获取节点的时候,都实现看看缓存区有没有,有的话直接返回,没有的话再读取
// 每次操作结束遍历该链表,将所有节点写回文件,但是不删除节点,而且相比使用vector缓存区加快了速度
class BPlusTree
{
public:
    BPlusTree(vector<Attribute> &type, string tableName); // 构造函数

    ~BPlusTree();

    bool insert(variableType key, int data); // 插入节点

    int getNum() { return num; } // 获得节点总数

    BNode *getRoot()
    {
        if (root)
            return root;
        BNode *node = ::getRoot(fileName);
        root = node;
        if (root)
            root->setTree(this);
        if (node != nullptr)
            rootOffset = -1;
        else
        {
            rootOffset = node->offset;
        }
        return node;
    }

    BNode *getMaxLeft()
    {
        BNode *node = ::getMaxLeft(fileName);
        if (node != nullptr)
            maxLeftOffset = -1;
        else
        {
            maxLeftOffset = node->offset;
        }
        return node;
    }

    int remove(variableType key); // 删除节点并且返回位置

    vector<int> rangeSearch(variableType pre, variableType last); // 范围查找

    vector<int> EqualSearch(variableType key); // 精确查找

    vector<int> BigSearch(variableType key); // 大于等于查找

    vector<int> SmallSearch(variableType key); // 小于等于查找

    int find(variableType key); // 查询节点是否存在

    void setRoot(BNode *ro)
    {
        root = ro;
        if (ro == nullptr)
        {
            rootOffset = -1;
            ::setRoot(fileName, -1);
        }
        else
        {
            rootOffset = ro->offset;
            ::setRoot(fileName, ro->offset);
        }
    }

    void setMaxLeft(LeafNode *le)
    {
        MaxLeft = le;
        if (le == nullptr)
        {
            maxLeftOffset = -1;
            ::setMaxLeft(fileName, -1);
        }
        else
        {
            maxLeftOffset = le->offset;
            ::setMaxLeft(fileName, le->offset);
        }
    }

    void clearUsed(); // 清空当前使用过的所有节点
public:
    vector<BNode *> used; // (老旧的缓存区)用于保存每次操作后读进来的内容,每次操作结束后释放,将他们写回去

private:
    int find(BNode *node, variableType key);

    int remove(BNode *node, variableType key);

    bool insert(BNode *node, variableType key, int data);

public:
    bool ifRepetition; // 是否可以存在重复
    string tableName;
    string fileName;
    int rootOffset;
    int maxLeftOffset;
    int order;
    BNode *root;
    LeafNode *MaxLeft;
    int num;
    vector<Attribute> type;
};
#endif // BPLUSTREE_BPLUSTREE_H
