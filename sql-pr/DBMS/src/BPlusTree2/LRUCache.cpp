#include "LRUCache.h"

LRUCache *cache = new LRUCache(1024);

vector<Attribute> getIndexTypeCache(string fileName)
{
    vector<Attribute> types;
    string indexTypeFile = fileName + ".type";
    fstream file(indexTypeFile.c_str(), ios::in);
    if (!file)
    {
        return types;
    }
    int len;
    file >> len;
    for (int i = 0; i < len; i++)
    {
        string name;
        int length;
        int ty;
        file >> name >> ty >> length;
        types.push_back(Attribute(name, ty, length));
    }
    return types;
}

void updateNodeCache(string fileName, BNode *node)
{
    vector<Attribute> types = getIndexTypeCache(fileName);
    fileName += ".index";
    fstream file(fileName, ios::in | ios::out | ios::binary);
    file.seekp(4096 * node->offset + 40, ios::beg);
    char buff[4097] = {0};
    int pos = 0;
    strcpy(buff + pos, to_string(node->offset).c_str());
    pos += 10;
    strcpy(buff + pos, to_string(node->parentOffset).c_str());
    pos += 10;
    strcpy(buff + pos, to_string(node->leftOffset).c_str());
    pos += 10;
    strcpy(buff + pos, to_string(node->rightOffset).c_str());
    pos += 10;
    if (node->getType() == LEAF)
    {
        strcpy(buff + pos, "1");
        pos++;
        strcpy(buff + pos, to_string(node->getNum()).c_str());
        pos += 9;
        for (int i = 0; i < node->getNum(); i++)
        {
            for (int j = 0; j < types.size(); j++)
            {
                Attribute ty = types[j];
                strcpy(buff + pos, node->Index[i].value[j].c_str());
                pos += ty.len;
            }
            strcpy(buff + pos, to_string(((LeafNode *)node)->Data[i]).c_str());
            pos += 10;
        }
    }
    else
    {
        strcpy(buff + pos, "0");
        pos++;
        strcpy(buff + pos, to_string(node->getNum()).c_str());
        pos += 9;
        for (int i = 0; i < node->getNum(); i++)
        {
            for (int j = 0; j < types.size(); j++)
            {
                Attribute ty = types[j];
                strcpy(buff + pos, node->Index[i].value[j].c_str());
                pos += ty.len;
            }
            strcpy(buff + pos, to_string(((IntervalNode *)node)->PointersOffset[i]).c_str());
            pos += 10;
        }
    }
    file.write(buff, 4096);
    file.close();
}

LRUCache::LRUCache(int _capacity) : capacity(_capacity), size(0)
{
    // 使用伪头部和伪尾部节点
    head = new DLinkedNode();
    tail = new DLinkedNode();
    head->next = tail;
    tail->prev = head;
}

void LRUCache::clear()
{ // 全部清空
    DLinkedNode *node = head->next;
    cache.clear();
    while (node != tail)
    {
        DLinkedNode *tmp = node->next;
        updateNodeCache(node->value->fileName, node->value);
        if (node->value->getType() == LEAF)
        {
            delete (LeafNode *)node->value;
        }
        else
        {
            delete (IntervalNode *)node->value;
        }
        delete node;
        node = tmp;
    }
    head->next = tail;
    tail->prev = head;
    size = 0;
}

void LRUCache::Remove(string key) // 某个节点需要被释放了,所以直接删除
{
    if (!cache.count(key))
    {
        return;
    }
    DLinkedNode *node = cache[key];
    removeNode(node);
    cache.erase(key);
    delete node;
    --size;
}

BNode *LRUCache::get(string key)
{
    if (!cache.count(key))
    {
        return nullptr;
    }
    // 如果 key 存在，先通过哈希表定位，再移到头部
    DLinkedNode *node = cache[key];
    moveToHead(node);
    return node->value;
}

void LRUCache::put(string key, BNode *value)
{
    if (!cache.count(key))
    {
        // 如果 key 不存在，创建一个新的节点
        DLinkedNode *node = new DLinkedNode(key, value);
        // 添加进哈希表
        cache[key] = node;
        // 添加至双向链表的头部
        addToHead(node);
        ++size;
        if (size > capacity)
        {
            // 如果超出容量，删除双向链表的尾部节点
            DLinkedNode *removed = removeTail();
            // 删除哈希表中对应的项
            cache.erase(removed->key);
            // 防止内存泄漏
            updateNodeCache(removed->value->fileName, removed->value);
            delete removed;
            --size;
        }
    }
    else
    {
        // 如果 key 存在，先通过哈希表定位，再修改 value，并移到头部
        DLinkedNode *node = cache[key];
        node->value = value;
        moveToHead(node);
    }
}

void LRUCache::addToHead(DLinkedNode *node)
{
    node->prev = head;
    node->next = head->next;
    head->next->prev = node;
    head->next = node;
}

void LRUCache::removeNode(DLinkedNode *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

void LRUCache::moveToHead(DLinkedNode *node)
{
    removeNode(node);
    addToHead(node);
}

DLinkedNode *LRUCache::removeTail()
{
    DLinkedNode *node = tail->prev;
    removeNode(node);
    return node;
}