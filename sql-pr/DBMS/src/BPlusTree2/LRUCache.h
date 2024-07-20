#ifndef LRUCACHE_HE_H
#define LRUCACHE_HE_H
#include "BNode.h"
#include "unordered_map"
struct DLinkedNode
{                 // 存放键值对
    string key;   // 记录键,类型为BPLustTree.name+offset
    BNode *value; // 记录对应的值
    DLinkedNode *prev;
    DLinkedNode *next;
    DLinkedNode() : value(0), prev(nullptr), next(nullptr) {}
    DLinkedNode(string _key, BNode *_value) : key(_key), value(_value), prev(nullptr), next(nullptr) {}
};
// 每次切换数据库都要清空缓存,我们使用extern全部函数,调用的头文件为CacheHead.h,里面有一个cache全局指针
// 每次关闭数据库或者切换数据库都要调用cache->clear()函数,还有输出exit;的时候也要调用
class LRUCache
{
private:
    unordered_map<string, DLinkedNode *> cache;
    DLinkedNode *head;
    DLinkedNode *tail;
    int size;
    int capacity;

public:
    DLinkedNode *removeTail();
    void moveToHead(DLinkedNode *node);
    void removeNode(DLinkedNode *node);
    void addToHead(DLinkedNode *node);
    LRUCache(int _capacity);
    void clear();
    void Remove(string key);
    BNode *get(string key);
    void put(string key, BNode *value);
};

#endif